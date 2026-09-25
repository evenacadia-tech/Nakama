# NAK-380 — Validierung der Erstprüfung 3 (Etappe 3: LRA mit 10 Hz, M1-Spanne)

| Merkmal | Wert |
|---|---|
| Ticket | NAK-380 (S25q), Etappe 3, Matrixzeilen M-21 bis M-41 |
| HEAD Beginn | `8308a3d80146f1119203127e927c1f746fcd4ec9` |
| HEAD Ende | `8308a3d80146f1119203127e927c1f746fcd4ec9` (gemessen unmittelbar vor dem Schreiben dieser Datei); alle zitierten Dateien ohne Diff gegen HEAD; Arbeitsstand sonst nur `eq-copilot/install/nakama-installer-v1.json` (Kanon, nicht Gegenstand) und die fremden untracked Ordner |
| Datum | 25.09.2026 |
| Modell | Opus 5.5, Effort max; frischer, lesender Validierer (nicht Bauer, Nacharbeiter oder Prüfer) |
| Gegenstand | Befunde D1 bis D4 der Erstprüfung 3 (Codex `gpt-6-astra` xhigh, Thread `01a0d8e7-d698-7352-84e9-538e3e8fe086`, Vorlage A, NEEDS_WORK, 4 Defekte, 0 Lücken) über `git diff b04dfb7c...8308a3d8 -- broker eq-copilot tools` |
| Prüfform | Statisch an der Quelle: Read, Grep, `git` nur lesend (rev-parse, status, diff, log, show, ls-files). Kein Bau, kein Test, kein cargo, kein Python-Lauf. Jede Aussage „bleibt grün“ oder „wird rot“ ist eine am Code nachgerechnete Kette, nicht gefahren. Zeilen in `docs/beweise/NAK-380.md` gelten am HEAD. |
| Ergebnis | **3 bestätigt (D1, D3, D4), 1 präzisiert (D2), 0 widerlegt.** Alle vier sind DEFEKT; kein Ausschluss der Vorlage greift. |

## Teil 1 — Befunde

### D1 — Perzentile unbelegter Bänder werden nicht je Feld auf NaN geprüft

**a) Maßstab.**

- M-38, `docs/beweise/NAK-380.md:913`, Zellen wörtlich:
  - Befund: „DSP-21 · Teilblockränder (starten↔stoppen)“
  - Zustand und Vorbedingung: „wie M-32; zweiter Lauf mit `zuruecksetzen()` bei 30 s“
  - Ereignis: „Snapshot“
  - Zusage: „Summe des Teilblockhistogramms je Band = ⌊segmente/8⌋ (ein unvollständiger Teilblock verfällt); nach `zuruecksetzen` beginnt der laufende Teilblock bei 0 Segmenten; ein Band mit weniger als 8 aktiven Segmenten trägt keine Perzentile (NaN, nie 0), `perzentileGueltig` erst mit mindestens einem Teilblock“
  - Reihenfolge und Frist: „`segmentInBaender` (`:500-503`), `zuruecksetzen` (`:277-336`)“
  - Test: „C++: A2 NEU `380/M-38 teilblock_raender`“
  - Rotbeweis und Etikett: „heute nicht messbar; Mutation: Rest als eigener Teilblock → +1 → rot“
  - Quelle: „R-380-6; `CLAUDE.md:77-79`, `:189-190`“
- Rotbeweisregel: §2 `NAK-380.md:75-76` „je Matrixzeile fällt der Rotbeweis an der Zeile, die die Zusage trägt, nicht an einem Nebeneffekt“; §6-Kopf `:788-789` „Das Etikett gilt je Satz der Zusage (R-313-12)“; `:796-797` „Ein Rotbeweis fällt an der Zeile, die die Zusage trägt, nie an einem Nebeneffekt.“
- Lehren aus §24.1: `:3562` D2 „P50 nur Band 220 … **DEFEKT** — M-01 „der P50 desselben Bandes“ gilt je belegtem Band“; `:3563` D3 „Bitzusage nur über die Schnittmenge … **DEFEKT** — M-01 „die 25 Bänder ohne Bin tragen kein Bit“ wird nicht an der Bitmap gemessen“.
- Präzisierung `docs/beweise/roh/NAK-380-etappe-3-auftrag.txt:16`: „Zusage je Element: sagt eine Zeile „je Band“, „je Fenster“, „je Sekunde“ oder „jeder Wert“ zu, prüft der Test jedes Element, keine Stichprobe (Lehre D2, Z1). Bit- und Validity-Zusagen werden an der Bitmap oder am Feld gemessen, nicht über einen Wertfilter (Lehre D3).“

**b) Nachmessung am HEAD.**

- Test `eq-copilot/plugin/tests/GoldenTestMain.cpp:610-614`:

  ```cpp
  const bool endlich = std::isfinite (snapshot.perzentilP10[(std::size_t) b])
                    && std::isfinite (snapshot.perzentilP50[(std::size_t) b])
                    && std::isfinite (snapshot.perzentilP95[(std::size_t) b]);
  if (bloecke == 0u && endlich) ++zuFrueh;
  if (bloecke > 0u && ! endlich) ++mitBlockOhneWert;
  ```

  Die Prüfung `:616-619` heißt „380/M-38 perzentile_erst_ab_einem_vollen_teilblock: jedes Band“. Sie prüft „alle drei endlich“ (UND-Verknüpfung), nicht „jedes Feld NaN“. Ein unbelegtes Band fällt nur, wenn alle drei Felder endlich sind.
- Produkt `eq-copilot/plugin/src/AnalyseEngine.cpp:828-830` füllt alle drei Felder mit NaN (`s.perzentilP10.fill (std::numeric_limits<double>::quiet_NaN());`, ebenso P50 und P95). `:838-839` `if (gesamt == 0) continue;` lässt unbelegte Bänder auf NaN, `:845-857` setzt belegte Bänder je Feld. Das Produkt erfüllt „NaN, nie 0“ heute.
- Kette, nachgerechnet: Mutation `AnalyseEngine.cpp:830` `s.perzentilP95.fill (0.0);`. Ein unbelegtes Band trägt dann P10 = NaN, P50 = NaN, P95 = 0; `endlich` ist falsch, `zuFrueh` bleibt 0. Belegte Bänder erhalten P95 in `:853-856`, `mitBlockOhneWert` bleibt 0. Flag-, Reset- und Speicherprüfung sind unberührt. `EqCopGoldenTest … --nak380 M-38` (`GoldenTestMain.cpp:699-706`) bleibt grün. Unbelegte Bänder mit Bins gibt es am Auswertepunkt: nach Reset und 1 s hat die Bassstufe 4 Segmente (D2 b); binlose Bänder haben 0.
- Wirkung einer 0 statt NaN: Der Knopfweg schreibt je Feld mit dem globalen Flag, `eq-copilot/plugin/src/DiagnoseAntwort.cpp:59` `return (gueltig && std::isfinite (v)) ? juce::var (v) : juce::var();` und `:151` `p95.add (zahl (m.perzentilP95[(size_t) b], m.perzentileGueltig));`. Sobald irgendein Band einen Teilblock hat, stünde für ein unbelegtes Band `p95_db` = 0 in der Festhalten-Datei. `Diagnose.cpp:208` verlangt P50 und P95 endlich und wäre nicht betroffen.
- Rotbeleg: `docs/beweise/roh/NAK-380-rot-M-38.txt:4` „NaN-Grenze und Speicherzahl blieben grün“, `:7` „Presence-Bit, NaN-Grenze und Speicherzahl blieben grün“. Für diesen Zusagesatz gibt es keine Mutation.

**c) Was wirklich bricht.** Der Zusagesatz „ein Band mit weniger als 8 aktiven Segmenten trägt keine Perzentile (NaN, nie 0)“ (`NAK-380.md:913`) wird nicht je Feld geprüft. Für diesen Satz kann deshalb kein Rotbeweis an der Zusagezeile fallen (`:75-76`, `:788-789`). Die Präzisierung `…-auftrag.txt:16` (jedes Element; Validity am Feld) ist verletzt; das ist dieselbe Lehre wie §24 D2 und D3. Die Invariante NaN-Ehrlichkeit (`CLAUDE.md:189-190`) ist im Produkt nicht gebrochen (`AnalyseEngine.cpp:828-830`). Gebrochen ist ihre Prüfung.

**d) Urteil.** **BESTÄTIGT.** Einordnung **DEFEKT**. Kein Ausschluss greift: Die Gegenprobe mutiert Produktcode, nicht das Prüfwerkzeug. Keine der entschiedenen Stellen (A-1 bis A-8, B-1 bis B-7, `NAK-380.md:1402-1467`; NAK-398 bis NAK-402, `docs/offene-punkte.md:612-616`) betrifft M-38.

**e) Schließende Regel.** Für jedes Band ohne vollständigen Teilblock verlangt M-38 jedes der drei Felder einzeln als NaN (ein Zähler je Feld), und eine formgültige Mutation je Feld (etwa `perzentilP95.fill (0.0)`) macht den Fall rot. Folgestellen: `GoldenTestMain.cpp:604-619`; `docs/beweise/roh/NAK-380-rot-M-38.txt` und eine neue Mutationsdatei; Rotbeweisspalte M-38 (`NAK-380.md:913`) in einer neuen Fassung mit einer Mutation je Zusagesatz.

### D2 — Die Vorbedingung von M-38 ist nicht hergestellt und nicht verriegelt

**a) Maßstab.**

- M-38 Vorbedingung (`NAK-380.md:913`): „wie M-32; zweiter Lauf mit `zuruecksetzen()` bei 30 s“. M-32 (`:907`): „AnalyseEngine 48 kHz; Weißrauschen L = R (§7 W4, σ = 0,1), 60 s, Block 512; `auswerten()`“. Signaltafel §7.2 (`:1123`): „W4 | Weißrauschen L = R, Gauß | σ = 0,1 | 0x3800006 | 60 s, 48 kHz | … | M-32, M-37, M-38 | A2“.
- starten↔stoppen: §6-Kopf `:824-825` „starten↔stoppen (Akkus über Fenster: Sitzungswechsel, Sprung, Abtastratenwechsel) M-38, …“; T-380-4 `:1280-1281` „ein unvollständiger Teilblock verfällt, `zuruecksetzen` leert ihn“.
- Präzisierung `…-auftrag.txt:15`: „Vorbedingung exakt: jeder Test stellt die in der Matrixzeile genannte Vorbedingung selbst her (Abtastrate, Blockgröße, Evidenzintervall über evidenzIntervallSetzen, Dauer, Saat, Pegelverlauf) und verriegelt daraus hergeleitete Zählgrößen (Zahl der Kurzzeitwerte je Sekunde, Zahl der Werte bis zur 60-s-Schwelle, Zahl der Teilblöcke je Fenster, Zahl der belegten Bänder) mit Formel im Kommentar, nie aus dem Lauf abgelesen (R-380-8; Lehre D1 aus §24).“
- Lehre §24.1 `:3561` D1: „… **DEFEKT** — M-01 Vorbedingung „Evidenzintervall 1 s“ | Intervall 1 s setzen, Framezahl hergeleitet verriegeln“.

**b) Nachmessung am HEAD.**

- `GoldenTestMain.cpp:571-572` `nak380M1Speisen (*engine, 30.0, …)`: 30 s statt 60 s, ohne `auswerten()`. `:588` `engine->zuruecksetzen();` bei 30 s. `:600-601` `nak380M1Speisen (*engine, 1.0, …)`: nach dem Reset 1 s statt bis 60 s. `:602` `engine->auswerten();`. 48 kHz, Block 512 und L = R stimmen (`:377-378`, `:389-390`, `:570`), die Saat ist `kW4Saat` (`:568`).
- floor/Rest wird nur vor dem Reset geprüft (`:575-586`), mit `n` aus dem Lauf (`segmentHistogrammSumme`, `:577`). Nach `:600` gibt es keine floor/Rest-Prüfung. Den Reset-Zustand selbst prüft der Fall direkt (`:589-598`: Segment- und Teilblockhistogramm 0, Rest 0, Restsumme 0,0 in allen 221 Bändern).
- Keine Verriegelung: Der Fall prüft weder die Samplezahl noch eine hergeleitete Segment- oder Teilblockzahl, anders als M-32 (`:450-453`) und M-37 (`:532-533`). Nachgerechnet aus `AnalyseEngine.cpp:70-71` (hop = n/2), `:108-109` und `:157-159` (erstes Segment nach n Samples, dann je hop) sowie `:214-217` (n = 16 384 / 4 096 / 2 048 für Bass, Mitten, Höhen). Es gilt Segmente = ⌊(N − n)/hop⌋ + 1. Alle W4-Segmente sind aktiv (σ = 0,1 ≈ −20 dBFS gegen die Schwelle −60 dB, `:13`, `:122-123`):
  - bei 30 s (N = 1 440 000): Bass 174 → 21 Teilblöcke, Rest 6; Mitten 702 → 87, Rest 6; Höhen 1 405 → 175, Rest 5;
  - bei 60 s (N = 2 880 000): Bass 350 → 43, Rest 6; Mitten 1 405 → 175, Rest 5; Höhen 2 811 → 351, Rest 3; der zweite Lauf mit Reset bei 30 s endet wieder bei 174 / 702 / 1 405;
  - nach Reset und 1 s (N = 48 000): Bass 4 → 0, Rest 4; Mitten 22 → 2, Rest 6; Höhen 45 → 5, Rest 5.
- Folge, nachgerechnet: Zählt die Engine kein Segment (etwa Mutation `AnalyseEngine.cpp:123` `warAktiv = false;`), sind alle Zähler 0. Dann gilt `:581` mit 0 = 0/8, `:589-595` ist leer, `:613-614` sehen nur NaN bei 0 Blöcken, `:620` vergleicht falsch mit falsch, `:624` bleibt gleich. M-38 bliebe grün. Ohne Verriegelung sind alle Beziehungsprüfungen des Falls auch leer erfüllbar.
- Den 1-s-Lauf nach dem Reset braucht der Fall trotzdem: Nur dort hat ein Band mit Bins weniger als 8 Segmente (Bass 4). Mit 30 s nach dem Reset wäre der NaN-Satz nur noch an binlosen Bändern beobachtbar.

**c) Was wirklich bricht.** Die Präzisierung `…-auftrag.txt:15` ist in beiden Teilen gebrochen. Die Dauer der Vorbedingung (W4 über 60 s nach `:907` und `:1123`, zweiter Lauf mit Reset bei 30 s) ist nicht hergestellt. Keine hergeleitete Zählgröße (Segmente, Teilblöcke, Rest je Stufe) ist mit Formel verriegelt; das ist dieselbe Lehre wie §24 D1. Nicht gebrochen ist der Satz „nach `zuruecksetzen` beginnt der laufende Teilblock bei 0 Segmenten“: Der Fall prüft ihn direkt am Zustand (`:589-598`). Eine floor/Rest-Prüfung nach dem Neustart trägt erst mit verriegelter Segmentzahl etwas bei (Rest 4 / 6 / 5 bei 0 / 2 / 5 Teilblöcken); ohne Verriegelung wäre auch sie leer erfüllbar.

**d) Urteil.** **PRÄZISIERT.** Die Fakten des Prüfers stimmen: 30 s, Reset, 1 s, kein floor(n/8) nach dem Neustart. Der tragende Mangel ist aber die fehlende Verriegelung der hergeleiteten Zählgrößen, nicht allein die fehlende Wiederholung von floor(n/8). Die Kurzzustände nach dem Reset müssen neben der Vorbedingung bestehen bleiben, sonst verlieren D1 und D3 ihren Prüfzustand. Einordnung **DEFEKT**. Kein Ausschluss greift.

**e) Schließende Regel.** M-38 fährt W4 wie §7.2 über 60 s und den zweiten Lauf mit Reset bei 30 s bis 60 s. An jedem Auswertepunkt verriegelt er die aus n, hop und Samplezahl hergeleiteten Segment-, Teilblock- und Restzahlen je Stufe mit Formel im Kommentar und prüft floor/Rest auch nach dem Neustart. Die zusätzlichen Kurzzustände für D1 und D3 werden in der Matrixzeile benannt. Folgestellen: `GoldenTestMain.cpp:565-628`; Vorbedingung von M-38 (`NAK-380.md:913`, neue Fassung mit den Auswertepunkten); `docs/beweise/roh/NAK-380-rot-M-38.txt`.

### D3 — `perzentileGueltig` wird nur auf der gültigen Seite geprüft

**a) Maßstab.** Letzter Satz der M-38-Zusage (`NAK-380.md:913`): „`perzentileGueltig` erst mit mindestens einem Teilblock“. Rotbeweisregel `:75-76`, Etikett je Satz `:788-789`. Präzisierung `…-auftrag.txt:16`: „Bit- und Validity-Zusagen werden an der Bitmap oder am Feld gemessen, nicht über einen Wertfilter (Lehre D3).“ Lehre §24.1 `:3563` (D3).

**b) Nachmessung am HEAD.**

- Test `GoldenTestMain.cpp:605` `bool irgendeinBlock = false;` und `:609` `irgendeinBlock = irgendeinBlock || bloecke > 0u;`; Prüfung `:620-621` `p.wahr (snapshot.perzentileGueltig == irgendeinBlock, "380/M-38 perzentileGueltig genau ab dem ersten vollen Teilblock", …)`. Einziger Auswertepunkt ist Reset plus 1 s. Dort haben Mitten (22 Segmente → 2 Teilblöcke) und Höhen (45 → 5) Teilblöcke (D2 b). `irgendeinBlock` ist deshalb in jedem Lauf wahr. Keine andere A2-Prüfung verlangt das Flag falsch; `:932` verlangt es für `pink-minus20` wahr.
- Produkt `AnalyseEngine.cpp:831` `bool irgendeins = false;`, `:838-840` `if (gesamt == 0) continue; irgendeins = true;`, `:860` `s.perzentileGueltig = irgendeins;`. Das Flag ist global: wahr, sobald irgendein Band einen Teilblock hat.
- Kette, nachgerechnet: Mutation `:860` `s.perzentileGueltig = true;` ergibt wahr == wahr, M-38 bleibt grün. Das Etikett `:621` („genau ab dem ersten vollen Teilblock“) verspricht mehr, als die Prüfung misst.
- Grenze, nachgerechnet: Der erste Teilblock nach dem Reset entsteht in der Höhenstufe nach 2 048 + 7·1 024 = 9 216 Samples (0,192 s); Mitten braucht 18 432, Bass 73 728. Bei 0,1 s (4 800 Samples) haben Höhen 3, Mitten 1 und Bass 0 Segmente. Dort gibt es keinen Teilblock, das Flag muss falsch sein. Der Vorschlag des Prüfers trägt.
- Leser gaten auf das Flag: `Diagnose.cpp:188` `if (m.perzentileGueltig)`, `DiagnoseAntwort.cpp:149-151`. Beide prüfen zusätzlich je Feld auf Endlichkeit (`Diagnose.cpp:208`, `DiagnoseAntwort.cpp:59`). Ein falsch wahres Flag wirkt deshalb heute kaum nach außen; die Zusage ist trotzdem ein eigener Satz der Matrixzeile.
- Rotbeleg: `rot-M-38.txt:7` führt das „Presence-Bit“ unter den grün gebliebenen Prüfungen. Eine Mutation des Flags gibt es nicht.

**c) Was wirklich bricht.** Der Satz „`perzentileGueltig` erst mit mindestens einem Teilblock“: Das „erst“ wird nie beobachtet. Ohne einen Zustand ohne Teilblock kann an dieser Zusagezeile kein Rotbeweis fallen (`:75-76`, `:788-789`). Die Präzisierung `:16` („am Feld gemessen“) ist formal erfüllt, weil das Feld gelesen wird, aber nur in einem Zustand. Die Lehre §24 D3 gilt sinngemäß.

**d) Urteil.** **BESTÄTIGT.** Einordnung **DEFEKT**. Kein Ausschluss greift.

**e) Schließende Regel.** M-38 wertet nach dem Reset einen Zustand ohne vollständigen Teilblock aus (etwa 0,1 s oder genau 9 215 Samples) und verlangt dort `perzentileGueltig == false` bei allen Feldern NaN; ab 9 216 Samples verlangt er `true`. Die Mutation „Flag immer wahr“ macht den Fall rot. Folgestellen: `GoldenTestMain.cpp:600-623` samt Etikett `:621`; `rot-M-38.txt` und eine neue Mutationsdatei; Rotbeweisspalte M-38.

### D4 — M-30 prüft nur das erste „kName = Wert“-Paar eines Registereintrags

**a) Maßstab.**

- M-30, `NAK-380.md:905`, Zellen wörtlich:
  - Befund: „DSP-25 · Kommentar und Register nennen §3.1“
  - Zustand und Vorbedingung: „`FeatureEngine.h:410` (Kommentar), `metriken-v1.json:149` (`nicht_gefuehrt`)“
  - Ereignis: „A5-Lauf“
  - Zusage: „der Kommentar nennt „EBU Tech 3342 §3.1 (≥ 10 Hz)“; der Registereintrag lautet „kLraHopZellen = 1 (EBU Tech 3342 §3.1: mindestens 10 Hz Abtastung der Kurzzeitlautheit)“; A5 prüft je `nicht_gefuehrt`-Eintrag der Form „kName = Wert“, dass der Code denselben Wert trägt“
  - Reihenfolge und Frist: „A5 vor dem Commit“
  - Test: „Python: A5 NEU `nak380_m30_nicht_gefuehrte_werte_stimmen`“
  - Rotbeweis und Etikett: „zwei Hälften: Text heute rot („§2.2“, Validierung Teil C `:379`); Prüfung Regressionswache (heute stimmen alle Einträge): Mutation Registertext „= 10“ bei Code 1 → A5 rot“
  - Quelle: „R-380-4 („der Kommentar nennt §3.1“)“
- Wachenbeleg §19.2 (`:2995`) belegt die Wache nur an `kLraHopZellen`: „`metriken-v1.json:143-149` und `FeatureEngine.h:410-411` tragen beide den heutigen Wert 10“.
- Präzisierung `…-auftrag.txt:16`: „… „jeder Wert“ zu, prüft der Test jedes Element, keine Stichprobe … Nennt eine Zeile mehrere Vertragsstellen oder Texte (M-30: Kommentar in FeatureEngine.h, Lautheit.h und Register metriken-v1.json), bekommt jede Stelle ihre eigene Teilprüfung und ihre eigene Mutation (Lehre D5).“ Lehre §24.1 `:3566` (D5) und Ursache (B) in §24.3 `:3627-3628`.

**b) Nachmessung am HEAD.**

- Codeindex `tools/eq-copilot/pruefe_v3_vertrag.py:2381-2387`: `re.findall(r"static\s+constexpr\s+(?:std::)?\w+\s+(k\w+)\s*=\s*([^;]+);", feature)`. Er liest nur `static constexpr` und nur `FeatureEngine.h`.
- Zerlegung `:2389-2393`: `treffer = re.match(r"^(k\w+)\s*=\s*(-?(?:\d+(?:\.\d*)?|\.\d+))(?=\s|,|\()", str(zeile))`, ohne Treffer `continue`. `re.match` ist am Zeilenanfang verankert und liefert je Eintrag genau ein Paar.
- Prüfung `:2405-2409` „nak380_m30_nicht_gefuehrte_werte_stimmen: jeder kName-Wert stimmt mit dem Code“. Das Etikett verspricht jeden Wert.
- Register `eq-copilot/schemas/v3/metriken-v1.json:412` in der aktuellen Fassung (`:20` `"aktuell": 20260926`): „kEreignisPlaetze = 64, kVerteilungPlaetze = 64 (Ressourcengrenzen, §48.1 'feste Obergrenzen')“. Geprüft wird nur `kEreignisPlaetze = 64`.
- Code: `eq-copilot/plugin/core/analysis/FeatureEngine.h:449` `static constexpr int kEreignisPlaetze = 64;` steht im Index; `FeatureEngine.h:124` `inline constexpr int kVerteilungPlaetze = 64;` steht nicht darin.
- Kette, nachgerechnet: Registerwert `kVerteilungPlaetze = 65`. `re.match` liefert nur `kEreignisPlaetze`. Die vier M-30-Prüfungen (`:2356-2360` Feature-Kommentar, `:2361-2364` Lautheit-Kommentar, `:2375-2379` `lra_zeile in nicht_gefuehrt`, `:2405-2409` Werte) bleiben grün; `--nak380 M-30` (`:2561-2566`) endet mit Exit 0. Auch der volle A5-Lauf fängt es nicht: `pruefe_metrikregister` liest nur `schwellen` und `ganzzahlige_schwellen` (`:2233-2251`), M-19 vergleicht `nicht_gefuehrt` der Fassung 20260925 mit 20260904 (`:2323-2343`), M-40 vergleicht nur die zwei Schwellenblöcke (`:2442-2447`).
- Würde nur die Zerlegung ergänzt, meldete die Prüfung `kVerteilungPlaetze: fehlt` (`:2394-2397`), weil der Index `inline constexpr` nicht liest. Beide Teile müssen geschlossen werden, wie der Prüfer schreibt.
- Rotbeleg `docs/beweise/roh/NAK-380-rot-M-30.txt:3` mutierte nur `kLraHopZellen = 1 -> 10`.

**c) Was wirklich bricht.** Der dritte Zusagesatz von M-30 für den Eintrag `metriken-v1.json:412`, dessen zweiter Wert ungeprüft bleibt. Ferner die Präzisierung `:16` („jeder Wert“, „jede Stelle ihre eigene Teilprüfung und ihre eigene Mutation“, Lehre D5) und das Etikett „heute stimmen alle Einträge“ (`:905`), dessen Wachenbeleg (`:2995`) nur `kLraHopZellen` trägt.

**d) Urteil.** **BESTÄTIGT.** Einordnung **DEFEKT**. Kein Ausschluss greift: Die Gegenprobe mutiert den Vertrag `metriken-v1.json`, nicht das Prüfwerkzeug; „Sabotage gegen Prüfwerkzeuge“ trifft nicht zu.

**e) Schließende Regel.** M-30 zerlegt jeden `nicht_gefuehrt`-Eintrag der aktuellen Fassung in alle „kName = Wert“-Paare und vergleicht jedes einzeln gegen einen Codeindex, der `static constexpr` und `inline constexpr` liest; ein Name ohne Codefund ist rot. Rotbeweis ist der zweite Wert von `:412` (64 → 65). Folgestellen: `pruefe_v3_vertrag.py:2381-2409`; `metriken-v1.json:412` nur für die Mutation, bytegleich zurück; `rot-M-30.txt` und eine neue Mutationsdatei; Rotbeweisspalte und Wachenbeleg von M-30 in neuer Fassung (append-only).

## Teil 2 — Gemeinsame Ursache

**D1, D2 und D3 teilen die Wurzel der §24-Ursache (A)** (`NAK-380.md:3625-3627`: „der M-01/M-02-Test bildet Vorbedingung, Zusage je Band oder Gruppe und Bitzusage der Matrixzeile nicht vollständig ab“). Der M-38-Test stellt die Vorbedingung nicht her und verriegelt keine Zählgröße (D2), prüft den NaN-Satz nicht je Feld (D1) und das Flag nur im wahren Zustand (D3). Dazu kommt Ursache (B) (`:3627-3628`: „Rotbeweis oder Prüfung decken nicht die Zusagezeile an allen genannten Stellen“): Von den vier Zusagesätzen tragen nur floor/Rest und Reset einen Rotbeweis (`rot-M-38.txt:3-4`, `:7`); NaN-Satz und Flagsatz haben keine Mutation.

**D4 teilt die Wurzel „je Vertragsstelle, je Wert eine Prüfung“** (Lehre D5, §24.3 Ursache (B)).

Die Lehren standen wörtlich im Etappenauftrag (`…-auftrag.txt:15-16`) und kehrten trotzdem wieder. Eine Punktkorrektur der vier Stellen genügt deshalb nicht.

**Durchsicht M-21 bis M-41 an der Quelle.**

| Zeile | Ergebnis | Beleg |
|---|---|---|
| M-21 | kein Befund: 48 kHz, Block 512, `evidenzIntervallSetzen (1.0)`, exakt 75 s über `fahreGenau`, 721 mit Formel verriegelt; keine Zufallsquelle, also keine Saat | `Sonde013DynamicsTest.cpp:289-306`, `:172-195` |
| M-22 | kein Befund: wie M-21, Werte 596/599/600 und Strom 3 019 200 verriegelt | `Sonde013DynamicsTest.cpp:309-340` |
| M-29 | kein Befund zur Wurzel: Vorbedingung wie M-21, 691 verriegelt. Satz 2 „kein nicht-endlicher Wert erreicht das Histogramm“ hat keine eigene Prüfung, ist im Produkt aber doppelt gesperrt; höchstens Härtung | `Sonde013DynamicsTest.cpp:393-412`; `Lautheit.h:100-101`, `:124` |
| M-24 bis M-27 | kein Befund: Referenz T-380-3 je Fall; Normtoleranz ±1 LU und §5-Referenz ±0,1 LU als getrennte Prüfungen; 48 kHz, Block 512, 80 bzw. 100 s, Intervall 1 s, L = R | `LoudnessGoldenTestMain.cpp:179`, `:277-329` |
| M-28, M-31 | kein Befund (gelesen, nicht vertieft) | `Sonde013DynamicsTest.cpp:414-474` |
| M-32 | kein Befund: jedes der 196 bzw. 69 Bänder, nicht endlich zählt als Fehler; Vorbedingung verriegelt | `GoldenTestMain.cpp:430-460` |
| M-33 bis M-35 | kein Befund: Median über die 17 Einbinbänder 200 bis 400 Hz; 14 400 000 Samples und 17 endliche Bänder verriegelt; Fenster aus der Matrix; Saaten gleich §19.3 | `GoldenTestMain.cpp:481-500`, `:643`, `:649`, `:655`; `Nak380Pruefsignale.h:14-19`; `NAK-380.md:3049-3051` |
| M-36 | verwandte Wurzel, Einordnung beim Dirigenten: Texte nur als Teilstring geprüft; „kommt in Wellen“ steht in keinem Test und keiner Referenz. Die 116-Hz-Spanne ist 0 dB, daher gelten `sp <= 6` und `spanne > 10.0` für jede Schwelle ≥ 0 gleich; der Wanderton wählt das Werkzeug über `dauerhaft`. Die Wache sieht nur Schwellen unter 0 (daher die Ersatzmutation `> -0.5`). Die Ersatzmutation fällt unter den Ausschluss, die fehlende Trennschärfe des Eingangs nicht zwingend | `GoldenTestMain.cpp:280-288`, `:359-360`; `Diagnose.cpp:212`, `:224-225`; `rot-M-36.txt:3`; `festhalten-referenz.json` („steht ruhig (Schwankung ~0 dB).“) |
| M-37 | gleiche Wurzel wie D2 in schwacher Form, Einordnung beim Dirigenten: je Band 221/221 und Abdeckungsnachbau geprüft; verriegelt sind nur die Samplezahl und die testseitig gerechnete Bandzahl, keine Segmentzahl. Beide Gleichheiten sind auch bei null gezählten Segmenten erfüllt; im selben A2-Lauf fängt M-32 ein leeres Histogramm | `GoldenTestMain.cpp:521-562`, `:409-419`, `:110`; `AnalyseEngine.cpp:983` |
| M-38 | D1 bis D3 | oben |
| M-39 | kein Befund: voller Bytevergleich mit Größe und `memcmp` | `BriefkastenTestMain.cpp:444-445`, `:457` |
| M-40 | kein Befund: C++-Konstante, beide Rust-Konstanten, Register `aktuell`, `seit` und Schwellen geprüft; Leserliste enthält 20260926 | `pruefe_v3_vertrag.py:2414-2447`; `vergleichbarkeit.rs:149`; `prepost.rs:206`; `telemetrie.rs:1187` |
| M-41 | kein Befund: Rotbeweis aus M-20 wiederverwendet; der Etappendiff berührt keinen Audio-, Sonden- oder Hostbridge-Pfad | `rot-M-41.txt`; `git diff --stat b04dfb7c...8308a3d8` |
| M-30, weiterer Eintrag | gleiche Wurzel wie D4, sofern „Klasse::kName = Wert“ als „Form kName = Wert“ gilt (Dirigent): `Vergleichspegel::kMindestSekunden = 0.4` scheitert an `^(k\w+)` und wird übersprungen; die Konstante liegt außerhalb des Index | `metriken-v1.json:417`; `pruefe_v3_vertrag.py:2390-2392`; `Vergleichspegel.h:42` |
| M-23 | nicht geprüft (Nacharbeit 0) | — |

**Randbefunde außerhalb von D1 bis D4, an der Quelle belegt.**

1. Nach `zuruecksetzen()` veröffentlicht die Engine bis zum nächsten `auswerten()` einen Snapshot mit P10/P50/P95 = 0,0 in allen 221 Bändern: `AnalyseEngine.cpp:333` `fertig = MessSnapshot {};` mit den Vorgaben `{}` in `AnalyseEngine.h:133-135`. `AnalyseEngine.cpp:338-340` setzt nur die LTAS-Felder auf NaN, `auswertenLeicht` (`:812-823`, `fuelleBasis` `:766-810`) lässt die Perzentile stehen. `perzentileGueltig` ist dort falsch (`AnalyseEngine.h:136`), und beide Leser gaten darauf (`Diagnose.cpp:188`, `DiagnoseAntwort.cpp:149-151`); nach außen wirkt es heute nicht. Der Wortlaut „NaN, nie 0“ der M-38-Zusage gilt an diesem Snapshot trotzdem nicht. Der Zustand ist vorbestehend (`b04dfb7c:eq-copilot/plugin/src/AnalyseEngine.h:133-135` gleich). Einordnung beim Dirigenten.
2. Der Kommentar `AnalyseEngine.h:130-131` („Band-Perzentile aus dem 1-dB-Pegelhistogramm der zuständigen Stufe (nur aktive Segmente; 1-dB-Quantisierung; NaN = Band ohne Segmente)“) ist seit T-380-4 falsch: Quelle ist `teilblockHistogramm` (`AnalyseEngine.cpp:834`), NaN heißt weniger als 8 aktive Segmente. Die Datei ist Ticketpfad der Etappe; der Diff hat die Stelle nicht nachgezogen.

**Was ein Nacharbeitsauftrag als Ursache tragen muss.**

1. **Ursache (A), für M-38 und bei M-37 mitzuprüfen:** Die Vorbedingung wird exakt nach Matrix und §7.2 hergestellt (W4 60 s; zweiter Lauf mit Reset bei 30 s bis 60 s). Die für D1 und D3 nötigen Kurzzustände nach dem Reset stehen zusätzlich in der Matrixzeile. An jedem Auswertepunkt werden Segment-, Teilblock- und Restzahlen je Stufe mit Formel verriegelt. Jeder Zusagesatz wird in einem Zustand geprüft, in dem er in beide Richtungen fallen kann: NaN je Feld; Flag falsch vor und wahr ab dem ersten Teilblock; floor/Rest vor und nach dem Neustart.
2. **Ursache (B):** Je Zusagesatz und je Wert gibt es eine eigene Prüfung und eine eigene formgültige Mutation. Für M-38 heißt das vier Sätze, neu mindestens die Feld- und die Flagmutation. Für M-30 heißt das jedes „kName = Wert“-Paar gegen einen Index aus `static constexpr` und `inline constexpr`. Prüfetiketten sagen nicht mehr, als sie prüfen (`GoldenTestMain.cpp:621`, `pruefe_v3_vertrag.py:2406`).
3. **Nachweis:** je Matrixzeile eine Zuordnung Zusagesatz → Prüfzeile → Mutation → rot ja/nein im Etappenabschnitt, damit die Wiederholung derselben Lehre am Beleg sichtbar ist.

## Übersicht

| Befund | Urteil | Einordnung | Schließende Regel in einem Satz | Folgestellen |
|---|---|---|---|---|
| D1 | BESTÄTIGT | DEFEKT | Für jedes Band ohne vollständigen Teilblock prüft M-38 P10, P50 und P95 einzeln auf NaN, und eine Mutation je Feld macht den Fall rot. | `GoldenTestMain.cpp:604-619`; `rot-M-38.txt` und Mutationsdatei; Rotbeweisspalte M-38 (`NAK-380.md:913`) |
| D2 | PRÄZISIERT | DEFEKT | M-38 stellt W4 über 60 s und den zweiten Lauf mit Reset bei 30 s her, verriegelt Segment-, Teilblock- und Restzahlen je Stufe und Auswertepunkt mit Formel und prüft floor/Rest auch nach dem Neustart. | `GoldenTestMain.cpp:565-628`; Vorbedingung M-38 (neue Fassung mit Auswertepunkten); `rot-M-38.txt` |
| D3 | BESTÄTIGT | DEFEKT | M-38 verlangt `perzentileGueltig == false` in einem Zustand ohne Teilblock (vor 9 216 Samples nach dem Reset) und `true` ab dem ersten Teilblock; die Mutation „Flag immer wahr“ macht den Fall rot. | `GoldenTestMain.cpp:600-623` samt Etikett `:621`; `rot-M-38.txt` und Mutationsdatei; Rotbeweisspalte M-38 |
| D4 | BESTÄTIGT | DEFEKT | M-30 prüft jedes „kName = Wert“-Paar jedes `nicht_gefuehrt`-Eintrags der aktuellen Fassung einzeln gegen einen Index aus `static constexpr` und `inline constexpr`; ein fehlender Name ist rot. | `pruefe_v3_vertrag.py:2381-2409`; `metriken-v1.json:412` nur als Mutation; `rot-M-30.txt` und Mutationsdatei; Rotbeweisspalte und Wachenbeleg M-30 |

## Nicht geprüft

- Nichts gefahren: kein Bau, kein Lauf von A2, A5, B9, B18 oder B30. Messwerte aus §27, §29 und den Rohdateien (etwa 116-Hz-Spanne 0 dB, 127 296 Byte, 2,600 LU) sind nicht reproduziert.
- Alle Ketten „bleibt grün“ (D1, D3, D4, Leerlauf bei D2 und M-37) und die Segmentzahlen sind statisch nachgerechnet.
- M-23 (Nacharbeit 0) und M-28 nur gelesen; die Toleranzherleitungen von M-32 bis M-35 (ν, Wilson-Hilferty) nicht nachgerechnet.
- Broker-Tests (A4) und generierte Dateien nicht gelesen, außer `broker/src/telemetrie.rs:1187`.
- `docs/**` ist nicht Gegenstand. Beobachtet ohne Prüfung: Die in §19.3 genannten Zielpfade der Etappe 3 (`docs/beweise/roh/NAK-380-etappe-3-*-altinhalt.txt` und `-ausgang.txt`, etwa `NAK-380.md:3040`) liegen am HEAD nicht im Index (`git ls-files`); ob die `rot-M-NN.txt` sie tragen, ist nicht geprüft.
- Kanon-Artefakte und die uncommittete Installer-Datei nicht angesehen.

FERTIG Validierung Erstprüfung 3, 3 bestätigt, 1 präzisiert, 0 widerlegt
