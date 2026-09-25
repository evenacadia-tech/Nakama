# NAK-380 Erstprüfung 4 — Validierung an der Quelle

| Feld | Wert |
|---|---|
| Ticket | NAK-380, Planschritt S25q, Etappe 4 (Ereignisdetektor nach §39.1, DSP-22) |
| HEAD Beginn | `1e4a7010915fc8a34765aa1fd6d1b10551cd2fd3` (gemessen 22:27:59 Uhr) |
| HEAD Ende | `1e4a7010915fc8a34765aa1fd6d1b10551cd2fd3` (gemessen 22:47:35 Uhr, nach dem Schreiben dieses Berichts; unverändert) |
| Datum | 25.09.2026 |
| Modell | Opus 5.5 (`claude-opus-5-5`), frischer lesender Validierer |
| Gegenstand | Prüfbericht der Erstprüfung 4 (Codex gpt-6-astra xhigh, Thread `01a0da2c-49f4-7e23-b226-44ba2e5f7f68`, URTEIL NEEDS_WORK, 7 Defekte; `docs/beweise/roh/NAK-380-erstpruefung-4-urteil.md`) über `git diff 21853aa1...1e4a7010 -- broker eq-copilot tools` |
| Prüfform | nur lesend (Read, Grep, `git rev-parse/status/log/show/diff/grep`); kein Bau, kein Lauf, kein Python, kein cargo; Zahlen am Code nachgerechnet, nicht gefahren; einzige Schreibaktion dieser Bericht |

Maßstab: `docs/beweise/NAK-380.md` §2 (R-380-2, R-380-7, R-380-8), §36.2, §5.1, §5.4, §6-Kopf, §6.3 in der Fassung §36.3/§38.2, §7.1 bis §7.3, §8.0, §8.1, §8.4, §24, §31; `docs/beweise/roh/NAK-380-etappe-4-auftrag.txt` (Präzisierungen :21-33) und `docs/beweise/roh/NAK-380-nacharbeit-e4-0-auftrag.txt` (:33-42). §35 bis §38 nur als Selbstbericht und Kontext.

Gemeinsame Referenzsätze (wörtlich, einmal zitiert, in Teil 1 mit Zeile verwiesen):

- **Regel §2** (`NAK-380.md:75-77`): „je Matrixzeile fällt der Rotbeweis an der Zeile, die die Zusage trägt, nicht an einem Nebeneffekt. Haben Befunde eine gemeinsame Ursache, ist die Ursache der Gegenstand."
- **§6-Kopf** (`:796-797`): „Ein Rotbeweis fällt an der Zeile, die die Zusage trägt, nie an einem Nebeneffekt."
- **R-380-8** (`:165-172`): „Jede Etappe belegt die neue Rechnung gegen eine Referenz außerhalb des eigenen Codes: … oder ein deterministisch im Test erzeugtes Signal mit bekannter Eigenschaft …; Prüfsignale entstehen im Test oder über einen Erzeuger, nie als Audiodatei im Repo."
- **§7.1** (`:1085-1087`): „Jedes Signal ist eine reine Funktion von Sampleindex, Parametern und Saat und entsteht im Test."
- **Lehren §24** (`:3561` D1 „M-01 Vorbedingung „Evidenzintervall 1 s““, `:3562` D2 „M-01 „der P50 desselben Bandes“ gilt je belegtem Band“, `:3566` D5 „M-17 „die vier Vertragsstellen unterscheiden““; Ursachen `:3625-3628`: „(A) der M-01/M-02-Test bildet Vorbedingung, Zusage je Band oder Gruppe und Bitzusage der Matrixzeile nicht vollständig ab …; (B) Rotbeweis oder Prüfung decken nicht die Zusagezeile an allen genannten Stellen“).
- **Lehre §31** (`:4430` „Vorbedingung exakt (Lehre D1 aus §24)“; `:4433` Z1 „Wache ohne Trennschärfe“; `:4439-4443` „Ab Etappe 4 verlangt jeder Bau- und Nacharbeitsauftrag je Matrixzeile eine Tabelle Zusagesatz → Prüfzeile → Mutation → rot ja/nein im Etappenabschnitt …; der Prüfer misst an ihr.“; `:4463-4465` „(A) der Test bildet Vorbedingung, Zusage je Feld und Flagzusage nicht vollständig ab …; (B) je Zusagesatz und je Wert eine eigene Prüfung und Mutation“).
- **Etappenauftrag** `:24`: „Vorbedingung exakt: jeder Test stellt die in der Matrixzeile genannte Vorbedingung selbst her (Abtastrate, Blockgröße, Dauer, Saat, Pegel, Signalform, konstruierte Spektren oder Historien über den Testzugang) und verriegelt daraus hergeleitete Zählgrößen … mit Formel im Kommentar, nie aus dem Lauf abgelesen“; `:25`: „Zusage je Element: sagt eine Zeile „je Klick genau eines“, „jedes Flussereignis“, „in jedem Fenster“ … zu, prüft der Test jedes Element, keine Stichprobe … Bit- und Flag-Zusagen werden am Bit oder Feld gemessen (M-60 beide Bits am Ereignis, M-63 `staerke` am serialisierten Wire-Text) … Eine Wache ohne Trennschärfe (Lehre Z1 aus §31) ist kein Beleg“; `:26`: „Rotbeweise formgültig: die Mutation darf keinen Längenriegel, keine Assertion, keine Panik und keinen Zugriff außerhalb eines Arrays auslösen“; `:27` (Schlusssatz): „Jede Toleranz und jede Zahl trägt im Testkommentar ihre Herleitung (Norm, Analytik, Statistik mit Zahl)."

---

## Teil 1 — je Befund

### D1 — M-62-Blockschleife liest über das Vektorende

**a) Zusage und Regel.** M-62 (`NAK-380.md:952`; §36.3 und §38.2 ändern M-62 nicht): Befund „DSP-22 · Grenze leert Vorframe und Historie (starten↔stoppen)“; Vorbedingung „nach E-380-13 selbstgeprüftes rosa Rauschen −20 dBFS, Seek bei 10 s (Grenzgrund Seek, Muster der B5-Grenzfälle)“; Ereignis „erster Hauptstufen-Frame nach der Grenze“; Zusage „kein Ereignis im ersten Frame nach der Grenze und keines, bevor die Historie wieder 32 aktive Frames trägt; Auskunft `flussBinVorgaengerGueltig()` (Testzugang) direkt nach der Grenze falsch“; Frist „`grenzeZiehen` (`Zeit.h:318-520`)“; Test „C++: B5 NEU `380/M-62 grenze_leert_detektor`“; Rotbeweis „heute nicht messbar (Binzustand entsteht erst); Mutation Leeren in `grenzeZiehen` weg → Auskunft wahr → rot“; Etikett heute nicht messbar. Dazu §7.1 `:1085-1087`, Etappenauftrag `:24` und `:26`, Regel §2 `:75-77`, Lehre D1 (§24 `:3561`, §31 `:4430`).

**b) Nachmessung am HEAD.**
- `eq-copilot/plugin/tests/AnalysisGoldenTestMain.cpp:1797` `auto x = sig::rosaMono (sig::kP2Saat, 0.1, 720000u);` — 720 000 Samples = 15 s · 48 000.
- `:1813` `while (strom < x.size())`; `:1815-1817` `for (std::uint32_t i = 0; i < 512u; ++i) audio[(std::size_t) i * 2u] = audio[(std::size_t) i * 2u + 1u] = x[(std::size_t) (strom + i)];`; `:1825` `b.sampleCount = 512u;`; `:1837` `strom += 512u;`.
- Rechnung: 720 000/512 = 1 406,25. Die Schleife läuft für strom = 0, 512, …, 1 406·512 = 719 872 (< 720 000). Im letzten Durchlauf liest `:1817` die Indizes 719 872 bis 720 383: 128 gültig, 384 (720 000 bis 720 383) hinter dem Vektorende. Danach strom = 720 384; die Engine bekam 720 384 Samples, die letzten 384 aus nicht bestimmtem Speicher. Keine Prüfung verriegelt die gespeiste Samplezahl (vgl. `:1369` `lauf->samples == 1440000u`, `:1761` `strom == 576000u`).
- Dieselbe Schleife steht schon im Zwischenstand (`git show 9125dbbc:…/AnalysisGoldenTestMain.cpp:1633-1635`) und in der Basisvariante der Gegenprobe (`docs/beweise/roh/NAK-380-etappe-4-m50-w1-altzaehler.txt`, Kopf „einzige Abweichung von der Endfassung: in M-62 `flussHatVorgaenger()` …“): Basisstand-Gegenprobe und alle M-62-Rotbeweise liefen mit diesem Lesen.
- Benachbarter Korpusläufer `nak380Korpuslauf`: `:1321-1322` `const auto anzahl = (std::uint32_t) std::min<std::uint64_t> ((std::uint64_t) block, samples - strom);`, `:1323` Kopie bis `anzahl`, `:1330` `b.sampleCount = anzahl;`, `:1340` `strom += anzahl;` — auf den Rest begrenzt (1 440 000 mod 512 = 256).
- Alle neuen Blockschleifen der Etappe (aus `git diff 21853aa1...1e4a7010 -- eq-copilot/plugin/tests/`):

| Schleife | Stelle | Signallänge | Rest mod 512 | Begrenzung | liest über das Ende |
|---|---|---|---|---|---|
| B5 Korpuslauf M-50 bis M-59 | `AnalysisGoldenTestMain.cpp:1319-1344` | 1 440 000 | 256 | `:1321-1322` min | nein |
| B5 Beobachtung I3 | `:1490-1524` | 1 440 000 | 256 | `:1492` min | nein |
| B5 M-61 (R1, 12 s) | `:1719-1756` | 576 000 | 0 | `:1721` min | nein |
| B5 M-62 | `:1813-1845` | 720 000 | 128 | keine | **ja, 384 Werte** |
| B16 Wirelauf M-63 (Stille, Sinus, I1), M-65 | `Sonde013EventWireTest.cpp:630-712` | 240 000 / 192 000 / 1 440 000 | 384 / 0 / 256 | `:632-633` min | nein |
| B18 M-47(a) | `Sonde013DynamicsTest.cpp:844` über `fahreGenau` `:175-195` | 480 000 | 256 | `:184-185` min | nein |
| B18 M-43 Allokationslauf | `Sonde013DynamicsTest.cpp:764` `strom + 512u <= w1.size()` | 96 000 | 256 | Abbruch vor dem Rest | nein (Rest nicht gespeist, s. Teil 2) |
| B22 M-68 | `Sonde013FingerprintGoldenTest.cpp:179-181` | 1 440 000 (Erzeugerfunktion) | 256 | `:181` min | nein |

**c) Was bricht.** §7.1 `:1085-1087` („reine Funktion von Sampleindex, Parametern und Saat“): 384 gespeiste Samples sind Speicherinhalt; die M-62-Vorbedingung „nach E-380-13 selbstgeprüftes rosa Rauschen“ (`:952`): die Selbstprüfung `:1798` deckt nur den Vektor; Etappenauftrag `:24` (Vorbedingung exakt, Dauer, Signalform). Das Lesen hinter dem Ende eines `std::vector` über `operator[]` ist in C++ undefiniertes Verhalten. Die Grenzzusage selbst wird nicht an der Überlesestelle gemessen: der übergelesene Strom 720 000 bis 720 383 liegt 239 744 Samples (4,99 s) hinter der Grenze 480 256 und hinter dem Zusagefenster bis 545 792 (`:1857-1864`); er geht aber in die Gesamtbedingung `ev.empty()` (`:1865`) ein. Ein Ereignis aus Fremdspeicher machte den Fall rot, nicht grün; wegen des undefinierten Verhaltens trägt der Lauf trotzdem keine belastbare Aussage (Prüfauftrag, DEFEKT-Klasse „undefiniertem Verhalten“).

**d) Urteil.** **BESTÄTIGT — DEFEKT.** Der Überlauf ist am HEAD Zeile für Zeile real und auf M-62 beschränkt (Tabelle). Kein Ausschluss greift: §35.8 Punkt 9 (`:5366-5370`) nimmt für M-62 nur „+14 dB und eine Delle −34 dB“ an, nicht die Schleifenform.

**e) Schließende Regel.** Jede Blockschleife begrenzt Kopierlänge, `sampleCount` und Fortschreibung auf min(Block, Rest) und verriegelt die gespeiste Samplezahl gegen die Signallänge. Folgestellen: `AnalysisGoldenTestMain.cpp:1813-1845` (Muster `:1321-1341`) und eine Vorbedingungsprüfung `strom == 720000u` in `:1849`; Rotbeweise `m62-leeren`, `m62-historie` neu fahren (`docs/beweise/roh/NAK-380-rot-M-62.txt`); Matrixzeile unverändert.

### D2 — M-48 prüft nur die Anzahl, nicht den Frame

**a) Zusage.** M-48 (`:938`; Fassung §36.3 `:5609-5612` und §38.2 `:6178-6179` „M-46, M-48, M-49: Matrixzahlen §6.3“, T_eff geltend T_min = 0,10·1 530 = 153,0 dB): Befund „DSP-22 · Spitzenwahl: lokales Maximum“; Vorbedingung „Testzugang: SF-Folge 0, 500, 450, 400 dB über vier Hauptstufen-Frames (48 kHz, Hop 42,67 ms), T_eff = 76,5“; Ereignis „vier Flussschritte“; Zusage „genau ein Ereignis am Frame mit 500; 450 liegt in der Sperrzeit und ist kein lokales Maximum, 400 (85,3 ms nach 500, außerhalb der Sperrzeit) ist kein lokales Maximum“; Test „C++: B18 NEU `380/M-48 spitzenwahl_lokales_maximum`“; Rotbeweis „heute rot (Basis: jede Überschreitung ein Ereignis, drei Ereignisse, nachgerechnet); nach dem Bau Mutation Bedingung SF(n) ≥ SF(n − 1) weg → 400 löst aus → rot“; Etikett heute rot. Dazu Etappenauftrag `:25` (Zusage je Element), Auftragsregel §31 `:4439-4443`, Lehre D2 (§24 `:3562`).

**b) Nachmessung.** `Sonde013DynamicsTest.cpp:632-642` `folge()`: `n += FeatureEngineTestzugang::detektorSchritt (*e, sf[i], (std::uint64_t) i * hopSamples);` und `return n;` — nur die Anzahl, Ereigniszeiten verworfen. `:874` `const int n = folge (historie (0.0, 0, 0.0), { 0.0, 500.0, 450.0, 400.0 });`, `:875-876` `pruefe (n == 1, "380/M-48 … ergibt genau ein Ereignis (am Frame mit 500)", …)` — „am Frame mit 500“ steht nur im Prüftext. Der Frame mit 500 hat Index 1, also `stromSample` 1·2 048 = 2 048. Die Bauertabelle §37.6 (`NAK-380.md:5896`) führt den ganzen Satz mit einer Prüfzeile und einer Mutation (`m48-lokal`).

Gegenprobe des Prüfers, am Code `Spektrum.h:789-890` nachgerechnet, Mutation `:851` `sf >= d.sfVorher` → `sf <= d.sfVorher`: frische Engine, `rahmenPeak` = `vorigerRahmenPeak` = 0 (`FeatureEngine.h:1601`, `:1631`) → `peakAus` falsch (`Spektrum.h:832`); Historie 32 × 0 über `historieSetzen` (`Sonde013DynamicsTest.cpp:349-356`), med = MAD = 0, T_eff = 153,0.
- i = 0, SF 0: kein Ereignis; `sfVorher` = 0.
- i = 1 (Strom 2 048), SF 500: 500 > 153, aber 500 ≤ 0 falsch → kein Ereignis; `sfVorher` = 500; Historie 31 × 0, 1 × 500 → med 0, MAD 0.
- i = 2 (Strom 4 096), SF 450: 450 > 153, 450 ≤ 500 wahr, Sperre frei (kein letztes Ereignis, `:846`) → Ereignis bei 4 096.
- i = 3 (Strom 6 144), SF 400: 400 ≤ 450 wahr, aber (6 144 − 4 096)·1 000 = 2 048 000 < 50·48 000 = 2 400 000 (`:848`) → gesperrt.
- n = 1 → `:875` grün, das Ereignis liegt am Frame mit 450. Bestätigt.
Zum Vergleich die Matrixmutation (Bedingung entfernt): i = 1 Ereignis bei 2 048, i = 2 gesperrt, i = 3 (6 144 − 2 048)·1 000 = 4 096 000 ≥ 2 400 000 → Ereignis → n = 2 → rot. Die Matrixmutation fällt, die gleichwertige formgültige Inversion nicht.

**c) Was bricht.** M-48 Zusagesatz „genau ein Ereignis am Frame mit 500“ (`:938`): der Frame hat keine Prüfung; Etappenauftrag `:25` und Auftragsregel §31 `:4439-4443` (je Zusagesatz eine Prüfzeile mit Mutation). Die R-380-2-Eigenschaft „lokales Maximum“ ist durch die Anzahl allein nicht getragen.

**d) Urteil.** **BESTÄTIGT — DEFEKT.** Kein Ausschluss (die A = 2·T_min-Variante §35.8 Punkt 8 ist in der Nacharbeit zurückgenommen, `:5896`).

**e) Schließende Regel.** Nennt eine Zusage einen Frame oder Zeitpunkt, prüft der Test den `stromSample` jedes Ereignisses gegen den aus Hop und Index hergeleiteten Wert. Folgestellen: `Sonde013DynamicsTest.cpp:632-642` (`folge()` liefert Ereigniszeiten), `:874-877` (genau ein Ereignis mit `stromSample == 2048`); zusätzliche Mutation `Spektrum.h:851` `sf >= d.sfVorher` → `sf <= d.sfVorher` mit Rohdatei (`docs/beweise/roh/NAK-380-rot-M-48.txt`); §37.6-Tabelle je Teilsatz (Frame, Sperrzeit, lokales Maximum) getrennt.

### D3 — M-63 prüft die Stärke nur als Bereich, nicht als Wert

**a) Zusage.** M-63 (`:953`): Befund „DSP-22 · Stärke endlich und begrenzt (schreiben↔lesen)“; Vorbedingung „Klick auf digitaler Stille (größter Fluss) und Korpus I1“; Ereignis „Ereignis, Serialisierung im `evidence_snapshot`“; Zusage „jedes Flussereignis trägt `staerke` = κ·(SF − med)/(T_eff − med) ∈ [3, 1000] (Nenner ≥ max(med, T_min − med) > 0, nachgerechnet), endlich; der Wiretext passiert Textriegel und Schema (`staerke_mad` 0 bis 1000, `eq-ipc-v3.schema.json:1280`)“; Frist „vor `ereignisAblegen`“; Test „C++: B16 NEU `380/M-63 staerke_begrenzt` (echter Serialisierer), B5“; Rotbeweis „heute rot (…); nach dem Bau Mutation Klammer 1000 weg → Klick auf Stille > 1000 → Schema rot“; Etikett nach §36.3 (`:5618-5620`) „Etikett der Stärkehälfte „heute nicht messbar (nachgerechnet)“, Zuordnungshälfte „heute rot““. Dazu R-380-8 (`:165-172`), Etappenauftrag `:25` (am Feld messen), Lehre D5 (§24 `:3566`, §31 `:4432`).

**b) Nachmessung.**
- Engine-Seite `Sonde013EventWireTest.cpp:662-664`: `if (! (std::isfinite (st) && st >= 3.0 && st <= 1000.0)) ++aus->flussMotorAusserhalb;`; Draht-Seite `:694` liest `staerke_mad`, `:704-705` dieselbe Bereichsprüfung; die Prüfungen `:741`, `:755`, `:782`, `:795`, `:800` verlangen `…Ausserhalb == 0` und Zähler > 0. Kein Vergleich gegen die Formel, kein Vergleich Draht gegen Engine.
- Zugang zu SF, med, T_eff: B16 definiert keinen `FeatureEngineTestzugang` (`git grep` findet ihn nur in `AnalysisGoldenTestMain.cpp:59`, `LoudnessGoldenTestMain.cpp:67`, `Sonde013DynamicsTest.cpp:221`); der Freund ist in `FeatureEngine.h:482` erklärt, T-380-11 (`NAK-380.md:1385-1387`) erlaubt je Testziel einen eigenen Zugang. B5 hat die Auskünfte `letzterFluss`/`naechsteSchwelle` (`AnalysisGoldenTestMain.cpp:1488-1516`), B18 `historieSetzen`, `detektorSchritt`, `letzteStaerke` (`Sonde013DynamicsTest.cpp:349-378`); `letzteStaerke` wird nirgends aufgerufen (nur Definition `:375-378`). Eine unabhängige Referenz mit med > 0 ist im bestehenden Zugang verfügbar.
- M-60-Einheit: `AnalysisGoldenTestMain.cpp:98-101` setzt die Historie auf 0 → med = 0; `:1679-1683` prüft 2κ = 6. Bei med = 0 sind (sf − med) und sf gleich; bestätigt.
- Mutation `Spektrum.h:872` `kFlussKappa * (sf - med) / nenner` → `kFlussKappa * sf / nenner`: SF ist eine Summe positiver Deltas (`Spektrum.h:730-735`), jeder Historienwert ist ≥ 0, also med ≥ 0; der Nenner ist > 0 (`:850-851`). Mutiert − Original = κ·med/(T_eff − med) ≥ 0, also liegt der mutierte Wert nach der Klemme `:872` in [Original, 1 000] ⊆ [3, 1 000]. Die Bereichsprüfung kann diese Mutation bei **keinem** Signal sehen, nicht nur bei I1.
- I1 (P2 −40 dBFS): med und MAD stehen in keiner gelesenen Quelle. Herleitbar nur: auf demselben P2-Hintergrund meldet die I3-Beobachtung T_eff 153,0 dB = T_min (`NAK-380.md:5835`, `:5842`, `:6185`), dort gilt max(med + 3·MAD, 2·med) ≤ 153,0, also med ≤ 76,5 dB. Ob med > 0 (nur dann weicht der Wert um κ·med/(153,0 − med) ab), ist nicht herleitbar.
- Unabhängige Referenz, am Code nachgerechnet: mit der M-46-Historie (`Sonde013DynamicsTest.cpp:817`, 16 × 160 und 16 × 200: med 180, MAD 20, T_eff 360) trägt SF 361 die Stärke 3·(361 − 180)/(360 − 180) = 3,016 67; unter der Mutation 3·361/180 = 6,016 67.
- Serialisierer (gelesen, nicht Ticketpfad): `eq-copilot/plugin/vertrag/NakamaEvidenz.cpp:526-535` lässt ein Ereignis mit `staerke` außerhalb [0, 1000] oder nicht endlich nicht auf den Draht und zählt es in `verloren`. Die Drahtprüfung `:704` sieht Werte über 1 000 und NaN deshalb nie (s. D5 und Teil 2, Z1).

**c) Was bricht.** M-63 Zusagesatz „jedes Flussereignis trägt `staerke` = κ·(SF − med)/(T_eff − med)“ (`:953`): der Wert hat keine Prüfung; R-380-8 (`:165-167`) für die Stärkerechnung nicht belegt; Lehre D5 (je Wert eine Prüfung). Bereich [3, 1000] und Endlichkeit sind an Engine und Draht geprüft.

**d) Urteil.** **BESTÄTIGT — DEFEKT** (verschärft: die Prüfung ist für jede formgültige Zählerverschiebung mit med ≥ 0 strukturell blind). Kein Ausschluss (§35.8 Punkt 10 betrifft den Nennersatz, nicht den Wert).

**e) Schließende Regel.** Jeder Wert, den eine Zusage als Formel nennt, wird gegen eine unabhängig gerechnete Referenz mit hergeleiteter Toleranz geprüft, der Drahtwert zusätzlich gegen den Engine-Wert desselben Ereignisses. Folgestellen: B18-Teilfall über `historieSetzen`/`detektorSchritt`/`letzteStaerke` (`Sonde013DynamicsTest.cpp:349-378`) mit med > 0 (Soll 3,016 67; Toleranz aus der float-Wandlung, halbe float-Stufe bei 3 ≈ 1,2·10⁻⁷); B16 `Sonde013EventWireTest.cpp:655-665` gegen `:690-707` Draht = Engine je Ereignis (Toleranz aus dem Zahlformat von `zahlJson`); Mutation `Spektrum.h:872` `(sf - med)` → `sf` → rot; Rohdatei `docs/beweise/roh/NAK-380-rot-M-63.txt`; Testspalte der Matrixzeile um B18 ergänzen (Dirigent).

### D4 — Rosa-Selbstprüfung fehlt vor den B16-Nutzern

**a) Zusage.** §7.1 (`:1099-1104`): „**Rosa:** Paul-Kellet-Filter (sieben Pole) auf Gauß-Weiß, auf den Soll-RMS normiert; der Test prüft seinen Erzeuger vor jedem Nutzer gegen E-380-13: für S(f) = C/f ist die integrierte Oktavbandleistung P[f, 2f] = C·ln 2 konstant, daher weichen benachbarte Oktaven von 31,5 Hz bis 16 kHz höchstens 1,0 dB voneinander ab." E-380-13 (§8.0 `:1394-1395`): „E-380-13 bindet §7.1, P1/P2 und alle Rosa-Nutzer an konstante integrierte Oktavbandleistung". §7.2 P2 (`:1125`): „wie P1 vor jedem Nutzer selbstgeprüft; Hintergrund von I1 bis I3, R1, K1 bis K5, M-62 \| M-57 bis M-59, M-61, M-62, M-103 bis M-107, M-109, M-112 \| B5, A2, B30“. §7.2 I1 (`:1130`): „P2 (−40 dBFS) plus Klicks“. M-63 Vorbedingung (`:953`): „Klick auf digitaler Stille (größter Fluss) und Korpus I1“.

**b) Nachmessung.** `Sonde013EventWireTest.cpp:735` `auto x = sig::rosaMono (sig::kP2Saat, 0.1, 240000u);`, danach sofort `:736-739` Stille und Klicks, `:740` gefahren; `:792` `auto i1 = sig::rosaMono (sig::kP2Saat, 0.01, 1440000u);`, `:793` `klicksEinsetzen`, `:794` gefahren. `git grep -n rosaGroessteOktavdifferenzDb -- eq-copilot/plugin/tests/` findet nur `Nak380Pruefsignale.h:329` (Definition) und `AnalysisGoldenTestMain.cpp:1389` (im B5-Helfer `nak380RosaGeprueft` `:1385-1403`). `rosaMono` (`Nak380Pruefsignale.h:313-320`) prüft nicht selbst. Die B5-Nutzer prüfen jeweils am unveränderten Puffer: `:1628-1629` (M-53), `:1413-1414` vor `:1415` (M-57 bis M-59), `:1709-1710` vor `:1711` (M-61), `:1797-1798` vor `:1801-1805` (M-62). Das B16-I1-Signal `:792` ist derselbe Aufruf wie B5 `:1413` (Saat, RMS, Länge); Bytegleichheit zwischen den Programmen ist nicht gemessen. Das Aufwärmsignal (0,1; 240 000 Samples) ist in dieser Länge nirgends geprüft (B5 prüft dieselbe Saat mit 576 000 und 720 000 Samples).

**c) Was bricht.** §7.1 „vor jedem Nutzer“ und E-380-13 „alle Rosa-Nutzer“ für beide B16-Nutzer. §7.2 nennt M-63 und B16 bei P2 nicht; die Bindung folgt aus §7.1 und E-380-13, nicht aus der Tafel. Die Stärkezusage von M-63 hängt nicht an der Oktavbandleistung des Hintergrunds; gebrochen ist die Vorbedingung, nicht die Zusage.

**d) Urteil.** **BESTÄTIGT — DEFEKT** (Vorbedingung, schwach). Kein Ausschluss: §35.8 Punkt 9 (`:5370-5373`) nimmt den Rosa-Hintergrund des Stille-Falls an („M-63 Stille nur um die Klicks“), nicht den Verzicht auf die Selbstprüfung.

**e) Schließende Regel.** Die E-380-13-Prüfung ist eine gemeinsame Funktion des Erzeugerkopfs, die jedes Programm vor jedem Rosa-Nutzer am unveränderten Puffer aufruft und als eigener Prüffall meldet. Folgestellen: `Sonde013EventWireTest.cpp:735` (vor `:736`) und `:792` (vor `:793`); der Helfer `AnalysisGoldenTestMain.cpp:1385-1403` wandert in `Nak380Pruefsignale.h`; Rotbeweis nach dem Muster `m53-rosa` (`NAK-380.md:5949`) auch in B16; §7.2 P2-Zeile um M-63/B16 ergänzen (Dirigent, docs).

### D5 — Verlustzähler in M-62 und M-63 nicht verriegelt

**a) Zählregel.** §7.3 (`:1154-1158`) wörtlich: „**Ereignisse** werden nach jedem gebauten Frame aus dem Ring gelesen (`ereignisAnzahlJetzt`, `ereignis (i)`) und danach mit `ereignisseEntnommen()` quittiert; der Verlustzähler `ereignisseVerworfen()` muss am Ende 0 sein, sonst ist die Zählung ungültig und der Fall rot." Derselbe Satz steht im Testkopf `AnalysisGoldenTestMain.cpp:1275-1277`. M-62 (`:952`) und M-63 (`:953`) wie oben. Ursache B (§24 `:3627-3628`, §31 `:4465`).

**b) Nachmessung** (`git grep -n -E 'ereignisseVerworfen|verloren'`):
- B5: Nullfall `:1369` `lauf->verworfen == 0u` (M-50 bis M-56); Impulsfall `:1449` `lauf->verworfen == 0u` (M-57 bis M-59); M-61 `:1761` `engine->ereignisseVerworfen() == 0u`; **M-62 `:1794-1871` ohne Aufruf**; Beobachtung `:1562-1563` nur Ausgabe (keine Zusage).
- B16: `nak380Wirelauf` reicht das Verlustdelta an den Draht (`:666-670`) und summiert die Draht-`verloren` (`:708`); **M-63 nur im Meldetext** (`:760-761`, `:789`, `:806-807`); M-65 `:842` `lauf->verloren == 0u` (Drahtsumme).
- B18: M-47(a) `:846` `e->ereignisseVerworfen() == 0u`.
- Tragweite: Ring drop-oldest mit 64 Plätzen (`Spektrum.h:933-940`, `FeatureEngine.h:554`), höchstens ein Ereignis je Detektorschritt (`Spektrum.h:856-883`). In M-62 setzt ein Verlust mehr als 64 Ereignisse zwischen zwei Entnahmen voraus; dann ist `ev` nicht leer und `:1865` ohnehin rot — dort verdeckt die fehlende Prüfung nichts, verletzt aber §7.3. In M-63 ist sie materiell: der Serialisierer zählt jedes nicht übertragene Ereignis (andere Epoche, `staerke` außerhalb [0, 1000] oder nicht endlich, vor dem Fensteranker, mehr als 64) in `verloren` (`NakamaEvidenz.cpp:510-553`). Ein Flussereignis, das nicht reist, fehlt den Drahtprüfungen `:704`/`:800`, die nur `flussDraht > 0` und „ausserhalb == 0" verlangen; nur der Stille-Fall prüft zwei bestimmte Klicks auf Anwesenheit (`:752-755`). Die Rohdatei `docs/beweise/roh/NAK-380-e4-n0-mutation-m63-klammer.txt` zeigt genau das: unter der Klammermutation „am Draht 1 (196608:16.747), verloren 2, nicht gebaut 4, Riegelfehler 0, Schemafehler 0".

**c) Was bricht.** §7.3 Satz 1 („muss am Ende 0 sein, sonst … rot“) in M-62 und M-63; in M-63 zusätzlich die Drahthälfte „jedes Flussereignis“ ohne vollständige Abdeckung.

**d) Urteil.** **BESTÄTIGT — DEFEKT** (M-62 formal, M-63 materiell). Kein Ausschluss.

**e) Schließende Regel.** Jeder Läufer, der Ereignisse zählt oder bewertet, verriegelt als eigene Prüfung `ereignisseVerworfen() == 0` am Laufende und am Draht die Summe `verloren == 0`, unabhängig von Zahl und Stärke. Folgestellen: `AnalysisGoldenTestMain.cpp:1849` oder eigener M-62-Teilfall; `Sonde013EventWireTest.cpp:713` (Engine-Zähler am Laufende in `Nak380Wirelauf` aufnehmen) und die M-63-Prüfungen `:741`/`:755`/`:782`/`:795`/`:800`; Rotbeweis M-63: `m63-klammer` fällt dann an der Verlustprüfung; für M-62 ist keine formgültige Produktmutation erreichbar (mehr als 64 Ereignisse in einem Frame sind unmöglich), die Prüfung ist dort Zählregel, kein Beleg; Rohdateien `NAK-380-rot-M-62.txt`, `NAK-380-rot-M-63.txt`.

### D6 — M-47(a) ohne Abschnittsnormierung

**a) Zusage.** M-47 (`:937`; §36.3/§38.2 ändern M-47 nicht): Befund „DSP-22 · Pegelbezug P0 und Aktivitätsgate“; Vorbedingung „(a) Weißrauschen L = R aus SplitMix64 und Box-Muller, Saat 0x3800001, je Abschnitt auf die feste Rahmenenergie normiert; springt bei 5 s von −90 auf −70 dBFS (beide Rahmenenergien unter dem Aktivitätsgate −60 dB, `Spektrum.h:448-451`); (b) Testzugang: Vorframe alle Bins −130 dBFS, Frame alle Bins −120 dBFS, als aktiv markiert“; Ereignis „Flussschritt“; Zusage „(a) kein Ereignis; (b) SF = 1530 · 10·log10((10⁻¹² + 10⁻¹⁰)/(10⁻¹³ + 10⁻¹⁰)) = 1530 · 0,0389 = 59,5 dB < 76,5 → kein Ereignis (nachgerechnet)“; Test „C++: B18 NEU `380/M-47 pegelbezug_p0`“; Rotbeweis „zwei Hälften: (a) Regressionswache (heute kehrt `Spektrum.h:477` vor dem Detektor zurück): Mutation Ereignisauslösung ohne Aktivitätsbedingung → Sprung um 20 dB löst aus → rot; (b) heute nicht messbar: Mutation P0 → 0 → SF = 1530 · 10 dB → Ereignis → rot“. §7.1 (`:1097-1099`): „**Rauschen:** gleichverteilt aus splitmix64 (wie heute), Gauß über Box-Muller aus zwei aufeinanderfolgenden Zügen derselben Saat, auf σ = 1 normiert und mit σ skaliert." Etappenauftrag `:24` (Pegel, Signalform).

**b) Nachmessung.** `Sonde013DynamicsTest.cpp:836` `weissMono (kM47Saat, 1.0, 480000u)`; `:837-839` `x[i] = (float) ((double) x[i] * (i < 240000u ? std::pow (10.0, -90.0 / 20.0) : std::pow (10.0, -70.0 / 20.0)));`. `Nak380Pruefsignale.h:248-255` `weissMono`: `v = (float) (sigma * g.naechstes());` — keine Normierung, keine Energieprüfung (anders als Rosa: analytisch normiert `:267-268`, RMS geprüft `AnalysisGoldenTestMain.cpp:1390-1394`). Saat `kM47Saat = 0x03800001ull` (`Nak380Pruefsignale.h:239`) gleich der Matrix. Gate `Spektrum.h:450-453` `const double gesamt = summeBereich (s, 0, bins) * binBreiteHz; const bool aktiv = gesamt > 0.0 && 10.0 * std::log10 (gesamt) > kAktivGateDb;` mit `FeatureEngine.h:501` `kAktivGateDb = -60.0`: Energie je FFT-Fenster der Stufe.
Tragweite (überschlagen, nicht gefahren): die Stichprobenenergie eines 5-s-Abschnitts aus 240 000 N(0, 1)-Werten weicht relativ um √(2/240 000) ≈ 0,29 % (≈ 0,013 dB, 1σ) vom Soll ab; je Hann-Fenster der Hauptstufe (4 096 Punkte) beträgt die relative Streuung der Energieschätzung ≈ √(3,89/4 096) ≈ 3,1 % (≈ 0,13 dB). Der Abstand zum Gate ist 30 dB (−90) bzw. 10 dB (−70). Die Zusage (a) hält ohne Normierung; die Rotbeweis-Mutation `m47-aktiv` (Ereignis ohne Aktivitätsbedingung) hängt nicht an der Normierung; Hälfte (b) nutzt konstruierte Spektren (`:851-858`) und ist unberührt.

**c) Was bricht.** Die Vorbedingung von M-47(a) „je Abschnitt auf die feste Rahmenenergie normiert“ (`:937`) und Etappenauftrag `:24`. Nicht gebrochen: die Zusage (a), die Hälfte (b), die generische Gauß-Regel §7.1 (σ = 1 aus Box-Muller, mit σ skaliert).

**d) Urteil.** **PRÄZISIERT — DEFEKT** (Vorbedingung, schwach). Der Befund stimmt in der Sache; zu präzisieren: er betrifft nur Hälfte (a) und nur die Vorbedingung, Zusage und Rotbeweis tragen unverändert. Kein Ausschluss.

**e) Schließende Regel.** Sagt eine Zeile „normiert“, stellt der Test die empirische Größe exakt her und prüft sie; der Abstand zum Gate je Fenster steht mit Herleitung im Kommentar. Folgestellen: `Sonde013DynamicsTest.cpp:836-839` (Abschnitte 0 bis 239 999 und 240 000 bis 479 999 je auf Σx²/n = 10⁻⁹ bzw. 10⁻⁷ skalieren und prüfen), Kommentar `:833-835`; `m47-aktiv` einmal wiederholen; Rohdatei `docs/beweise/roh/NAK-380-rot-M-47.txt`.

### D7 — neue Gleichheitstoleranzen ohne Herleitung

**a) Präzisierung.** Etappenauftrag `:27` (Schlusssatz): „Jede Toleranz und jede Zahl trägt im Testkommentar ihre Herleitung (Norm, Analytik, Statistik mit Zahl)." §6-Kopf `:760-761`: „*Zusage* einen prüfbaren Satz mit Maß, Einheit und Toleranz und bei Messwerten die Referenz nach R-380-8". Lehre L1 (§24 `:3568`). Matrix: M-42 (Fassung §38.2 `:6174-6176`; Sollwert 79,96 dB), M-47(b) (`:937`, 59,5 dB), M-60 (`:950`, „ein reines Peakereignis trägt `staerke` = Crest über Schwelle in dB (`Spektrum.h:697-699`); lösen Fluss und Peak im selben Frame aus, entsteht genau ein Ereignis mit beiden Bits“) — keine dieser Zeilen nennt eine Toleranz.

**b) Nachmessung** (`git grep -n -E '1\.0e-(9|6|4|5)'`, dazu `git diff -U0 21853aa1...1e4a7010` der vier Testdateien):

| Stelle | Toleranz | Vergleich | Herleitung der Toleranz im Kommentar |
|---|---|---|---|
| `Sonde013DynamicsTest.cpp:688` | 1,0·10⁻⁹ | M-42 d = 10: SF (double) gegen `pegelMitP0(−20) − pegelMitP0(−120)` | nein; `:665-670` leitet nur den Sollwert 79,9568 dB her |
| `Sonde013DynamicsTest.cpp:858` | 1,0·10⁻⁶ | M-47(b): SF (double) gegen 1 530·(`pegelMitP0(−120)` − `pegelMitP0(−130)`) | nein; `:849-850` nur Sollwert |
| `AnalysisGoldenTestMain.cpp:1669` | 1,0·10⁻⁴ | M-60(a): float `staerke` gegen double Crest − 12 dB | nein; `:1652-1658` nur Sollwert 14,0206 |
| `AnalysisGoldenTestMain.cpp:1683` | 1,0·10⁻⁵ | M-60(b): float `staerke` gegen 2κ = 6 | nein |
| `AnalysisGoldenTestMain.cpp:1769` | 1,0·10⁻⁹ | M-61: Vorframe gegen −100 dB | qualitativ `:1766-1768`: „10*log10(10^(-10)) ist in double nicht zwingend bitgenau -100; ein Vorframe aus dem letzten aktiven Fenster laege um Dutzende dB darueber." |

`Sonde013DynamicsTest.cpp:930` (Treffer derselben Suche) ist Etappe-2-Code und ein Eingangswert, keine Toleranz. Art: alle fünf sind Gleichheitstoleranzen einer Rechnung (double bzw. float-Wandlung), keine Zusagetoleranzen der Matrix. Überschlag (nicht gefahren): M-42 Differenz zweier Pegel ≤ 100 dB mit Rundung von wenigen ulp(100) ≈ 1,4·10⁻¹⁴ → Budget ≈ 10⁻¹³ dB; M-47(b) Summe über 1 530 Bins mit Einzelfehler ≈ 3·10⁻¹⁴ → ≲ 10⁻¹⁰ dB; M-60(a) float bei 14,02: halbe Stufe 2⁻²¹ ≈ 4,8·10⁻⁷; M-60(b): T_min = 0,10·1 530 rundet in double auf 153,0, 3·306/153 = 6 ist in double und float exakt → exakter Vergleich möglich.

**c) Was bricht.** Etappenauftrag `:27` für die vier Toleranzen ohne Herleitung (M-42, M-47(b), M-60 a und b); M-61 trägt eine qualitative Herleitung (Grund und Trennabstand), kein Rechenbudget. Keine Matrixzusage und keine Referenz bricht: die Sollwerte sind analytisch (R-380-8 erfüllt), die Toleranzen enger als jede Matrixzahl.

**d) Urteil.** **BESTÄTIGT — DEFEKT** (Kommentarebene, schwach). Kein Ausschluss.

**e) Schließende Regel.** Jede Gleichheitstoleranz nennt im Kommentar ihr Rechenbudget (Operationen, Wertebereich, ulp bzw. float-Stufe) und liegt nachvollziehbar darüber; exakt darstellbare Werte werden exakt verglichen. Folgestellen: `Sonde013DynamicsTest.cpp:688`, `:858`; `AnalysisGoldenTestMain.cpp:1669`, `:1683` (exakt), `:1766-1769` (Zahl ergänzen); keine Verhaltensänderung, Rotbeweise unberührt.

---

## Teil 2 — Gemeinsame Ursache und Durchsicht

**Ursache A** (der Test bildet Vorbedingung und Zusage der Matrixzeile nicht vollständig ab; §24 `:3625-3627`, §31 `:4463-4464`):
- **D2: ja.** Die Hilfsfunktion `folge()` (`Sonde013DynamicsTest.cpp:632-642`) reduziert jede Zusage auf eine Zahl; der Frame der Zusage kommt nicht zurück.
- **D6: ja.** Die Vorbedingung „normiert“ wird nicht hergestellt und nicht gemessen.
- **D1: ja, mit eigener unmittelbarer Ursache.** Unmittelbar ist es eine kopierte Blockschleife ohne Restblock: M-61 (`AnalysisGoldenTestMain.cpp:1719-1756`), M-62 (`:1813-1845`) und die Beobachtung (`:1490-1524`) haben eigene Schleifen neben `nak380Korpuslauf` (`:1301-1349`), der den Seek schon kann (`:1302`, `:1326-1327`). Wurzel A zeigt sich darin, dass M-62 als einziger Läufer die gespeiste Samplezahl nicht gegen die Signallänge verriegelt (vgl. `:1369`, `:1761`); diese Prüfung hätte 720 384 statt 720 000 gezeigt.

**Ursache B** (je Zusagesatz, je Wert und je Zählregel eine eigene Prüfung mit Referenz; §24 `:3627-3628`, §31 `:4465`):
- **D3: ja** (Wert ohne Referenz), **D5: ja** (Zählregel §7.3 ohne Prüfung), **D7: ja** (Toleranz ohne Herleitung).

**D4: ja** — eine Vorbedingung, die das Programm B16 nicht selbst herstellt. Die E-380-13-Prüfung ist ein B5-Helfer (`AnalysisGoldenTestMain.cpp:1385-1403`); der Kopf `Nak380Pruefsignale.h` liefert nur die Rechenfunktion (`:329-369`), `rosaMono` (`:313-320`) prüft nicht. B16 bindet den Kopf und nutzt den Erzeuger, ruft die Prüfung aber nicht auf.

**Durchsicht M-42 bis M-72 an der Quelle** (nur Belegtes):

1. **Blockschleifen (D1-Wurzel):** Tabelle in D1 — nur M-62 liest über das Ende. Hinweis: der M-43-Allokationslauf (`Sonde013DynamicsTest.cpp:764`) speist 187 Blöcke = 95 744 statt 96 000 Samples, der Kommentar `:752` sagt „2 s“; kein Überlesen, keine Matrixvorbedingung (M-43 `:933` sagt nur „im Heap in `vorbereiten`“), kein Befund. Die Etappe-2-Schleifen (`AnalysisGoldenTestMain.cpp:933`, `:1006`; `Sonde013EventWireTest.cpp:515`) lesen aus Erzeugerfunktionen, nicht aus Vektoren.
2. **Verlustzähler (D5-Wurzel):** verriegelt M-50 bis M-56 (`:1369`), M-57 bis M-59 (`:1449`), M-61 (`:1761`), M-65 (`Sonde013EventWireTest.cpp:842`), M-47(a) (`Sonde013DynamicsTest.cpp:846`); nicht verriegelt M-62 und M-63 (D5). Hinweis M-65: geprüft ist die Drahtsumme; der Engine-Zähler am Laufende wird nicht gelesen (nach dem 93. Snapshot bleiben 1 440 000 − 93·15 360 = 11 520 Samples, ≤ 6 Hauptstufen-Frames; ein Verlust ist dort nicht erreichbar).
3. **Rosa-Nutzer (D4-Wurzel):** B5 geprüft vor jedem Nutzer M-53 (`:1628-1629`), M-57 bis M-59 (`:1413-1414`), M-61 (`:1709-1710`), M-62 (`:1797-1798`); ungeprüft B16 M-63 (`Sonde013EventWireTest.cpp:735`, `:792`, D4) und die Beobachtung (`AnalysisGoldenTestMain.cpp:1475`, ohne Zusage, Hinweis). Der Rückgabewert von `nak380RosaGeprueft` sperrt die Folgeprüfungen nicht (`:1414`, `:1629`, `:1710`, `:1798`); die Selbstprüfung ist ein eigener Prüffall und macht das Programm rot — kein Befund.
4. **M-57:** je Klick geprüft — Zuordnung je Ziel `:1419-1433`, `:1453` `n == soll && genauEiner == (int) ziele && fremd == 0`; 112 Klicks aus der Formel (`:1639`, `Nak380Pruefsignale.h:424-430`).
5. **M-58/M-59:** I2 je Paar (`:1428` `passt (klicks[2 * z]) || passt (klicks[2 * z + 1])`), I3 je Klick (`:1650`, `paarweise` falsch); 150 ms = 7 200 Samples mit `static_assert` über N_H + Hop (`Nak380Pruefsignale.h:438-440`).
6. **M-61:** Fenster [331 904, 336 000] = [7,0 s − 4 096/48 000 s, 7,0 s] (`:1760`); „keines in den stationären Abschnitten“ über `ev.size() == 1u` für den ganzen Lauf (`:1759`); Vorframe und Historie am Testzugang (`:1769-1781`).
7. **M-64:** je Rate über alle Blockgrößen identisch (`:3582-3606`, `alleGleich`), Schranke [Impuls − N_H, Impuls] (`:3600-3602`); bestehender Fall.
8. **M-65:** jedes Fenster — alle gebauten Snapshots (`Sonde013EventWireTest.cpp:827-834`), `nichtGebaut == 0` (`:835`), 93 und 89 hergeleitet (`:816-823`).
9. **M-49:** 1 Hop und 2 Hops bei 48 und 44,1 kHz, Zeiten hergeleitet (`Sonde013DynamicsTest.cpp:883-897`); Sperrrechnung `Spektrum.h:846-848`.
10. **M-45/M-46:** Historien konstruiert (`:796-800` 32 × 0; `:812-821` 16 × 160 und 16 × 200 → med 180, MAD 20; `:822` `teff == 360.0`); beiderseits der Schwelle geprüft.
11. **M-44:** 28 × 1,0 und 4 × 11,0 (`:785` `historie (11.0, 4, 1.0)`), exakt geprüft (`:787`).
12. **M-43:** K = 1 666/1 530/765 und k-Bereiche aus der Herleitung (`:696-715`), 2·K·8 B (`:709`); Ring 252/232/118 aus 2·w_max + 2 (`:716-741`), Hinlänglichkeit gegen das direkte Maximum (`:742-749`).
13. **M-50:** Golden 0 (`AnalysisGoldenTestMain.cpp:1621`); Altzähler 18 in `docs/beweise/roh/NAK-380-etappe-4-m50-w1-altzaehler.txt` („ALTZAEHLER W1 = 18 Ereignisse in 30 s“), Rotlauf gegen 18 in `docs/beweise/roh/NAK-380-rot-M-50.txt:19-21` (gelesen, nicht gefahren).
14. **M-68:** `kNak380F1Bytes[76]` (`Sonde013FingerprintGoldenTest.cpp:160-169`) bytegleich zur Rohdatei `docs/beweise/roh/NAK-380-etappe-4-m68-f1-ausgang.txt:27-31`; `git log 21853aa1..1e4a7010 -- …/Sonde013FingerprintGoldenTest.cpp` nennt nur `9125dbbc` — kein Nachzug in der Nacharbeit.
15. **M-70:** sieben geführte Schwellen einzeln (`tools/eq-copilot/pruefe_v3_vertrag.py:2555-2563`, `:2659-2685`: genau ein Codefund, Wert = Register = geltend, Datei `FeatureEngine.h`), vier Versionsstellen einzeln (`:2621-2640`); am Code `FeatureEngine.h:89` `20260927u`, `:181` −100.0, `:199` 125.0, `:204` 32, `:207` 3.0, `:210` 1.0, `:224` 0.10, `:229` 50.0; `broker/src/coordinator/prepost.rs:206` und `vergleichbarkeit.rs:149` `20260927`; `eq-copilot/schemas/v3/metriken-v1.json:20` `"aktuell": 20260927`.
16. **Toleranzen (D7-Wurzel):** alle neuen Gleichheitstoleranzen der vier Testdateien sind die fünf aus D7; `1.0e300`, `1.0e30`, `1.0e-300` sind Startwerte bzw. Logarithmusschutz. Die Rosa-Toleranz 0,5 dB trägt eine Herleitung (`AnalysisGoldenTestMain.cpp:1379-1384`), 1,0 dB ist der E-380-13-Wert.
17. **Zeit je Ereignis (D2-Wurzel):** `folge()` dient M-45, M-46, M-48, M-49; nur M-48 nennt einen Frame. M-57 bis M-61 prüfen `stromSample`.
18. **Stärkewert (D3-Wurzel):** kein Test prüft einen Stärkewert bei med > 0; `letzteStaerke` (`Sonde013DynamicsTest.cpp:375-378`) ist ungenutzt.

**Zusätzlicher Befund Z1 (Wurzel B, mit D5 verbunden).** Die M-63-Drahthälfte „der Wiretext passiert Textriegel und Schema (`staerke_mad` 0 bis 1000)“ ist für die Stärke eine Wache ohne Trennschärfe: der Serialisierer lässt Werte außerhalb [0, 1000] und nicht endliche Werte nicht auf den Draht (`NakamaEvidenz.cpp:526-535`), ein Schemafehler an `staerke_mad` kann nie entstehen. Der Matrix-Rotbeweis „Klammer 1000 weg → Klick auf Stille > 1000 → Schema rot“ (`NAK-380.md:953`) tritt nicht ein: `docs/beweise/roh/NAK-380-e4-n0-mutation-m63-klammer.txt` meldet „Schemafehler 0“; die Drahtprüfung `Sonde013EventWireTest.cpp:755` fällt am fehlenden Klick (Verlust, „verloren 2“), also an einem Nebeneffekt (Regel §2 `:75-76`; Lehre Z1 §31 `:4433`; Etappenauftrag `:25`). Einordnung: DEFEKT (schwach) gegen die Rotbeweisregel; er schließt sich mit D5 — die Verlustprüfung ist die Stelle, an der diese Mutation zusagegemäß fällt — und mit einer berichtigten Rotbeweisspalte (Dirigent).

**Was ein Nacharbeitsauftrag als Ursache tragen muss (statt Punktkorrektur):**
- **Ursache A:** Jeder Fall stellt die Vorbedingung seiner Zeile her und verriegelt sie am Lauf — gespeiste Samplezahl = Signallänge, normierte Größe gemessen, jede in der Zusage genannte Größe (Anzahl und `stromSample`/Frame) kommt aus dem Lauf zurück und wird geprüft; Blockschleifen laufen über einen gemeinsamen, auf den Rest begrenzten Läufer.
- **Ursache B:** je Zusagesatz, je Wert und je Zählregel eine eigene Prüfung mit unabhängiger Referenz und hergeleiteter Toleranz — Stärke als Wert (med > 0) und Draht = Engine; `ereignisseVerworfen() == 0` und Draht-`verloren == 0` in jedem Läufer; Rechenbudget jeder Toleranz im Kommentar; keine Wache ohne Trennschärfe (Z1).
- **Erzeugerbindung:** Vorbedingungen eines Erzeugers (E-380-13) als gemeinsame Funktion im Erzeugerkopf, die jedes Programm vor jedem Nutzer aufruft (B16 heute; laut §7.2 P2 später A2 und B30).
- **Durchsicht als Auftragsteil:** alle Läufer M-50 bis M-65 gegen dieselbe Liste (Restblock, Samplezahl, Verlustzähler, Rosa-Selbstprüfung, Zeit je Ereignis, Wert gegen Referenz), nicht nur die sieben Stellen.

---

## Übersicht

| Befund | Urteil | Einordnung | schließende Regel in einem Satz | Folgestellen |
|---|---|---|---|---|
| D1 M-62 liest 384 Werte über das Vektorende | BESTÄTIGT | DEFEKT | Jede Blockschleife begrenzt Kopie, `sampleCount` und Fortschreibung auf min(Block, Rest) und verriegelt die gespeiste Samplezahl. | `AnalysisGoldenTestMain.cpp:1813-1845`, `:1849`; `NAK-380-rot-M-62.txt` (m62-leeren, m62-historie) |
| D2 M-48 ohne Frameprüfung | BESTÄTIGT | DEFEKT | Nennt eine Zusage einen Frame, prüft der Test den `stromSample` jedes Ereignisses gegen den hergeleiteten Wert. | `Sonde013DynamicsTest.cpp:632-642`, `:874-877`; Mutation `Spektrum.h:851` `>=` → `<=`; `NAK-380-rot-M-48.txt` |
| D3 M-63 Stärke nur als Bereich | BESTÄTIGT (verschärft) | DEFEKT | Jeder Formelwert wird gegen eine unabhängige Referenz mit Toleranz geprüft, der Drahtwert gegen den Engine-Wert. | B18 über `Sonde013DynamicsTest.cpp:349-378`; `Sonde013EventWireTest.cpp:655-707`; Mutation `Spektrum.h:872`; `NAK-380-rot-M-63.txt` |
| D4 Rosa-Selbstprüfung fehlt in B16 | BESTÄTIGT | DEFEKT (schwach) | Die E-380-13-Prüfung ist eine Kopffunktion, die jedes Programm vor jedem Rosa-Nutzer aufruft. | `Sonde013EventWireTest.cpp:735`, `:792`; Helfer aus `AnalysisGoldenTestMain.cpp:1385-1403` nach `Nak380Pruefsignale.h`; §7.2 P2 (Dirigent) |
| D5 Verlustzähler in M-62/M-63 nicht verriegelt | BESTÄTIGT | DEFEKT (M-62 formal, M-63 materiell) | Jeder Läufer verriegelt `ereignisseVerworfen() == 0` und Draht-`verloren == 0` als eigene Prüfung. | `AnalysisGoldenTestMain.cpp:1849`; `Sonde013EventWireTest.cpp:713`, `:741`-`:800`; `NAK-380-rot-M-62.txt`, `-M-63.txt` |
| D6 M-47(a) ohne Abschnittsnormierung | PRÄZISIERT (nur Vorbedingung von (a); Zusage und Rotbeweis tragen) | DEFEKT (schwach) | Sagt eine Zeile „normiert“, stellt der Test die empirische Größe exakt her und prüft sie. | `Sonde013DynamicsTest.cpp:833-839`; `NAK-380-rot-M-47.txt` |
| D7 Toleranzen ohne Herleitung | BESTÄTIGT | DEFEKT (Kommentar, schwach) | Jede Gleichheitstoleranz nennt ihr Rechenbudget, exakt darstellbare Werte werden exakt verglichen. | `Sonde013DynamicsTest.cpp:688`, `:858`; `AnalysisGoldenTestMain.cpp:1669`, `:1683`, `:1766-1769` |
| Z1 (Durchsicht) M-63-Schemahälfte ohne Trennschärfe | neu, Validierer | DEFEKT (schwach), schließt mit D5 | Eine Wache, die der Serialisierer vorab erzwingt, ist kein Beleg; die Mutation fällt an der Verlustprüfung. | `Sonde013EventWireTest.cpp:755`; `NakamaEvidenz.cpp:526-535` (gelesen); Rotbeweisspalte M-63 `NAK-380.md:953` (Dirigent) |

## Nicht geprüft

- Kein Bau, kein Lauf, keine Speicherprüfung: ob das Überlesen in M-62 eine Zugriffsverletzung auslöst und welchen Inhalt es liest (Heaplage), ist nicht bestimmt.
- med und MAD an I1 und im Stille-Fall von M-63: in keiner gelesenen Quelle; hergeleitet ist nur med ≤ 76,5 dB auf dem P2-Hintergrund.
- Zahlformat von `zahlJson` (für die Toleranz Draht gegen Engine): nicht gelesen.
- Bytegleichheit des I1-Signals zwischen B5 und B16 (gleicher Aufruf): nicht gemessen.
- M-66, M-67 (Rust), M-69, M-71, M-72, der Peakpfad Zeile für Zeile gegen die Basis, die Kalibrierfolge (NAK-405 ausgeschlossen) und die übrigen Punkte 1 bis 9 des Prüfauftrags: nicht Gegenstand dieses Auftrags, nicht geprüft.
- Rohdateien nur die genannten gelesen (`NAK-380-etappe-4-m50-w1-altzaehler.txt`, `NAK-380-rot-M-50.txt`, `NAK-380-etappe-4-m68-f1-ausgang.txt`, `NAK-380-e4-n0-mutation-m63-klammer.txt`); ihre Laufwerte sind Selbstbericht.
- Die Überschläge in D6 und D7 sind Größenordnungen aus der Analytik, nicht gefahren.

FERTIG Validierung Erstprüfung 4, 6 bestätigt, 1 präzisiert, 0 widerlegt
