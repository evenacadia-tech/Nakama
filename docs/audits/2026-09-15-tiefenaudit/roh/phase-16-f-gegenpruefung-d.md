# Phase 16, Paket F: unabhängige Gegenprüfung von Paket D

Datum: 18.09.2026. Prüfer: Paket F, vom Finder D getrennt. Produktquelle ausschließlich aus `C:\na-audit16-aff2d818`, HEAD `aff2d8188f33a9525aec1869449773bfdbe305a6`. Dieser Bericht prüft D-01, D-02 und die neuen Komponentennachweise zu T3-15-05, T3-14-02 und T3-15-08. Keine Produktänderung, keine Installation, kein FL- oder Produktionspipelauf, keine Sicherheitsprüfung. Die Vergabe zentraler IDs bleibt bei der Hauptsession.

## Ergebnis und Beweisgrenze

| Gegenstand | Urteil nach Gegenangriff | Grund |
|---|---|---|
| D-01 / F07: Auto-Gain auf Mono | **Als eng gefasste neue LÜCKE/medium bestätigt:** fehlende Mono-Fachregel; kein nachgewiesener Verstoß gegen die ausdrücklich definierte Stereoformel. | Der angenommene Mono-Bus gibt nur L aus. Ein alleiniger right- oder side-Shelf verändert diesen Ausgang ohne Auto-Gain nicht, erzeugt aber einen deutlich negativen Ausgleich. Die bekannte Kanalbelegung geht nicht in die Ableitung ein. |
| D-02 / F08: Gitter über Nyquist bei niedrigen Raten | **Herabgestuft: benötigt Produkt-/Supportentscheidung; keine neue T3-ID.** | R4 nennt bereits 44,1 kHz als kleinste unterstützte Rate. Dass der Eingangs-Sanitycheck niedrigere Raten nicht abweist, beweist ihre Ausführbarkeit, aber keine weitergehende DSP-Supportzusage oder Pflicht zur Ablehnung. Die ausgeführte Formel entspricht dem festgelegten Gitter. |
| T3-15-05: alter Trim beim Wiedereinschalten | **Bestehender Defekt durch echte Komponentenausführung bestätigt.** | Das bisher nachgerechnete Maximum von +13,41 dB entsteht am wirklichen float-Ausgang; keine Rampen- oder Fadeformel wurde als Implementierungsersatz benutzt. |
| T3-14-02: Remove und Neubelegung ohne Audio dazwischen | **Bestehender Defekt jetzt auch über echte erfolgreiche Transaktionen bestätigt.** | Remove und Belegen committen mit Revision 1 und 2; der neue Slot erbt trotzdem die alte dynamische Auslenkung und produziert bei Stille einen Nachklang. |
| T3-15-08: unbeteiligte Slot-Historie beim Topologiewechsel | **Bestehende LÜCKE mit echter Audiofolge bestätigt; keine zweite ID.** | Eine neutrale Änderung an Slot 5 startet den unveränderten warmen Slot 0 neu. E-8 erklärt den Kaltstart, beseitigt aber weder die gemessene Wirkung noch die fehlende Regel für unveränderte Slots. |
| F12: pauschaler Kaltstart nach Laden | **Für identischen Same-Instance-Ladestart widerlegt.** | Echter `ladestart` plus erzwungene Wiederpublikation erhält bei gleicher Topologie warmes Audio und Tap bitgenau. Eine neue Vorbereitung ist ein anderer Lebenszyklus und braucht einen ausdrücklich gewählten Vergleichsbezug. |

Der Finder stimmte der Herabstufung von D-02 nach dem Gegenangriff zu. Die realen Messwerte bleiben im [D-Bericht](phase-16-d-dsp.md) und seinen Nachweisen erhalten; aus dem alternativen Teilgitter wird kein nachträglicher Sollwert gemacht.

## Eigene Prüfung der ausführbaren Nachweise

Gelesen wurden [dsp_probe.cpp](phase-16-nachweise/d/dsp_probe.cpp), [CMakeLists.txt](phase-16-nachweise/d/CMakeLists.txt), [run.ps1](phase-16-nachweise/d/run.ps1), [ergebnisse.txt](phase-16-nachweise/d/ergebnisse.txt), [manifest.json](phase-16-nachweise/d/manifest.json), die genannten Produktstellen und die jeweiligen Zusagen/Gegenentscheidungen in `docs/beweise/SONDE-015.md`.

Die CMake-Datei linkt die unveränderten DSP-, Parameter- und Transaktionsimplementierungen. `publish` ruft den wirklichen DTO-Validator auf (`dsp_probe.cpp:36-38`); `run` gibt echte float-Puffer an `DspKern::verarbeite` und liest dessen `postCommitted`-Tap (`:40-45`). Die Frequenzauswertung nimmt eine gemessene Impulsantwort und eine komplexe DFT (`:68-70`), keine abgeschriebene RBJ-/SVF-Antwort. Für Auto-Gain wird das produktseitige Frequenzgitter benutzt: damit prüft der Test die ausgeführte Filterantwort gegen die Ableitung, **nicht unabhängig die fachliche Wahl dieses Gitters**.

Eigener Wiederholungslauf ab **2026-09-18 00:24:15 UTC**, nach Abgleich aller 15 im D-Manifest genannten Produktquellhashes und des Binaryhashes:

```powershell
& 'C:\na-audit16-aff2d818\build-d\Release\audit16d.exe'
```

- Binary-SHA256: `C5B02B9EFD3CF471301F52516E10018543C3A00AFF2479519C261931665D5FF7`.
- Auditaufrufer-SHA256: `7A6194A10C66669096BA458097614D8BC560E660817D08050912317B53BD3271`.
- Auditaufrufer zuletzt geschrieben: `2026-09-18T00:23:42.6500526Z`; Binary: `2026-09-18T00:23:47.4316330Z`.
- Kein Hashunterschied zu den 15 Produktquellen; der Status ihrer DSP-/State-/Sonde-Verzeichnisse in der isolierten Kopie war leer.
- Eigener Lauf: `HARNESS_FINISHED`, Exit 0. Die unten genannten Zahlen wurden erneut ausgegeben. D bestätigte anschließend, keine weitere Harness-/Binaryänderung vorzunehmen.

Das ist ein unabhängiger Quellen-/Orakelcheck plus Wiederholung derselben echten Komponentenausführung. Es ist **kein zweiter unabhängig implementierter DSP** und kein SondeProcessor-/VST3-/FL-Nachweis. Die erfolgreiche Reproduktion darf nicht als bestandener Produktregressionstest oder Gesamt-PASS bezeichnet werden.

## D-01: starker Gegenangriff und verbleibende Lücke

**Ausgangslage und Wirkung.** 48 kHz, Mono, `eq_enabled=true`, ein belegter und aktivierter Low-Shelf bei 20 kHz, Q = 1/√2, +12 dB, Modus `right`, Mix 1, Trims 0. Ein 250-Hz-Sinus mit Amplitude 0,25 läuft eine Sekunde; RMS wird über die letzte halbe Sekunde gemessen. Ohne Auto-Gain liegt der Monoausgang bei rund 0 dB Änderung, mit Auto-Gain bei **−9,177564 dB**. Im Modus `side` sind es **−7,839905 dB**. Der ausgewertete Ton liegt weit unter der Shelfkante und der Nyquistgrenze; der Befund hängt nicht an der D-02-Niedrigratenfrage.

**Tatsächlicher Weg.** `eq-copilot/plugin/sonde/SondeProcessor.cpp:311-332` nimmt einen identischen Mono-Ein-/Ausgang ausdrücklich an; `:338` und `:396-398` geben die wirkliche Kanalzahl an den DSP-Kern. Host-/Statewerte gelangen über `eq-copilot/plugin/state/NakamaTransaktion.cpp:340-347` und `:753-762` nach `eq-copilot/plugin/dsp/DspKern.cpp:216-231`. `DspProgramm.cpp:162-163` kennt beim Programmbau DTO und Samplerate, aber keine Kanalzahl. Die Ableitung mittelt bei `DspProgramm.cpp:125-129` weiterhin zwei gedachte Ausgangsseiten. Im Audioweg wird Mono nach L und R dupliziert (`DspKern.cpp:893-895`); ausgegeben wird bei einem Kanal ausschließlich L (`:1079-1085`). Die Kanalmodi wirken dagegen vor diesem Ausgang auf beide internen Seiten (`:507-512`, `:596-601`), und der abgeleitete Gain wird auf L und R angewandt (`:715-730`).

**Erreichbarkeit.** Ein Mono-Layout ist heute ein zugelassener Produkt-/Hosteingang, Stufe 1. Die vorbereitete Belegung kommt im Nachweis aus dem gebauten DTO/API, Stufe 2; im Produkt kann sie aus geladenem Zustand beziehungsweise Transaktion stammen. Die Gen-Bedienung zum Belegen bleibt NAK-306. Weder ein heutiger FL-Monobus-Handgriff noch das Herstellen genau dieses belegten Programms allein über die vorhandene Oberfläche wurde ausgeführt. Die technische Mono-Annahme ist nicht mit einem beobachteten FL-Nutzerfall gleichzusetzen.

**Gegenargument: R4 ist absichtlich materialfrei und darf danebenliegen.** Das ist richtig. `SONDE-015.md:1622-1624` definiert die Stereo-/M/S-Näherung ausdrücklich und lässt den M/S-Kreuzterm bewusst weg. Eine einzelne korrelierte Stereoaufnahme ist deshalb kein Orakel für eine falsche Ableitung. Die Messung `channels=2` im Harness berichtet zudem nur L-RMS (`dsp_probe.cpp:65`), nicht die summierte Stereoenergie; sie darf nicht als solcher Stereoenergie-Gegenbeweis verwendet werden. Die Stereoformel des Codes wird von D-01 nicht widerlegt.

Die engere Lücke bleibt: Mono ist ein angenommenes Ausgabeformat, und die Ableitung erhält die bekannte Zahl der tatsächlich ausgegebenen Kanäle nicht. Für den belegten Einzelbandfall ist die gesamte ausgegebene Mono-Übertragung ohne Auto-Gain neutral; seine Absenkung lässt sich nicht mit unbekannter Korrelation zwischen zwei ausgegebenen Kanälen erklären. `SONDE-015.md:1603-1616` fordert einen Ausgleich aus der statischen Kurve und für eine flache Kurve 0 dB, legt aber die Bedeutung dieser Kurve beim angenommenen Mono-Layout nicht fest. Das begründet die **fehlende Mono-Fachregel**, keinen schon eindeutig spezifizierten Zahlenfehler. R4s normale Stereo-Näherung bleibt eine vorgesehene Regel.

**Keine Duplikation.** NAK-283 F04 begrenzt Busse auf Mono/Stereo; es definiert die Mono-Ableitung nicht. T3-15-09 behandelt extreme Beträge und die unzutreffend hergeleitete Vertragsgrenze ±120 dB; die vorliegenden Werte sind moderat. Die bekannte 0,45·fs-Frequenzkappung ist eine andere Funktion.

**Kleinster kohärenter Folgeschritt.** Eine Mono-Regel für das tatsächliche, geordnete Übertragungsprogramm festlegen und danach Kanalbelegung in Berechnung und Neubau einbeziehen. **Nicht pauschal alle right-/side-Bänder ignorieren:** ein vorheriges right-Band kann den internen R-Weg verändern, den ein späteres M/S-Band wieder nach L mischt. Deshalb Einzelbandfälle und gemischte geordnete Kaskaden prüfen. Persistente Kanalwünsche erhalten; die Stereo-/identischen-Mid-Side-Goldens müssen unverändert bleiben. Der gezielte Regressionstest braucht Mono mit `right`/`side`, Stereo-Gegenfälle, einen Layoutwechsel und eine Kaskade `right` vor `mid`; das genaue Soll folgt der gewählten Mono-Regel.

## D-02: warum der neue DSP-Befund nicht trägt

**Bestätigte Ausführung.** `SondeProcessor.cpp:237-238` behandelt endliche Raten über 0 bis 768 kHz als gültig und reicht sie bei `:252-260` in den Kern. Bei 32 kHz, Bell 14 kHz, +12 dB, Q 4, liegen vier der 121 Gitterstellen über Nyquist. Die tatsächliche Ableitung ist **−0,397113060814 dB**; der Mittelwert nur der unter Nyquist liegenden gemessenen Punkte ergibt **−0,234376456735 dB**. Q 1 ergibt **−1,447171024841** beziehungsweise **−0,940514679344 dB**. Bei 22,05 kHz sind elf Punkte oberhalb Nyquist; bei 44,1/48 kHz keiner. Der eigene Lauf bestätigt außerdem, dass die Ableitung mit der vollständigen gemessenen Impulsantwort auf dem **festgelegten** Gitter übereinstimmt.

**Entscheidender Gegenweg.** `docs/beweise/SONDE-015.md:1620` schreibt ein samplerateunabhängiges 20-Hz–20-kHz-Gitter vor und nennt ausdrücklich die kleinste unterstützte Rate 44,1 kHz. `DspProgramm.cpp:76-82` und `:99-102` setzen genau dieses Gitter um. Die alternative Teilgitterrechnung ist eine mögliche andere Fachregel, kein vorhandenes Soll. `DspProgramm.cpp:214-215` beziehungsweise `DspFilter.h:109-114` begrenzen die eigentliche Bandfrequenz zusätzlich auf 0,45·fs; der Nachweis zeigt keine instabile Filterberechnung.

Ein Sanitycheck am Prozessoreingang ist keine Zusage, für jeden durchgelassenen Wert dieselben Goldens zu erfüllen. Die gefundenen DSP-Matrizen und `DspGoldenTestMain.cpp:569` prüfen 44,1/48/96/192 kHz. Die Analyse hat eigene 22,05-/32-kHz-Gegenfälle (`AnalysisGoldenTestMain.cpp:2919`, `GoldenTestMain.cpp:749`); daraus folgt nicht automatisch eine zusätzliche DSP-Auto-Gain-Zusage. Der fehlende untere Filter im gelesenen VST3-Wrapper belegt ebenfalls Annahme, nicht Support. Eine Anweisung, solche Raten zwingend abzuweisen, wurde weder von F noch vom Finder in den gezielt gelesenen Quellen gefunden.

**Endurteil.** F08 benötigt eine konkrete Produkt-/Supportentscheidung zur heute angenommenen, außerhalb der beschriebenen DSP-Matrix liegenden Rate. D-02 wird **nicht als neue LÜCKE oder DEFEKT aufgenommen**. Bei gewünschter Unterstützung niedriger Raten müssen Gitterende und Gewichtung definiert werden; bei verbindlicher Begrenzung muss der Umgang mit Anfragen außerhalb der unterstützten Menge bestimmt werden. Der 32-kHz-Test samt 44,1-kHz-Gegenfall steht dafür bereit. Bis dahin folgt weder eine Kappungsreparatur noch eine neue T3-ID aus diesem Lauf. T3-15-09s Extremwertproblem und T3-15-10s Decramping-Prüfauftrag bleiben davon getrennt.

## T3-15-05: tatsächlicher Ausgang statt Formeldoppelung

`dsp_probe.cpp:52-60` fährt den originalen Kern bei 48 kHz mit DC 0,5: EQ an und Output-Trim +24 dB, 1024 Samples Einschwingen; aus und 512 Samples; Trim bei ausgeschaltetem EQ auf 0 dB und 512 Samples; wieder an und 512 Samples messen. Die ersten drei Intervalle sind länger als die 256-Sample-Rampen beziehungsweise -Fades. Defaults und Validierung sind echt; es gibt keine belegten Bänder, Detektoren oder Shelf-Einschwingvorgänge, die den Ausschlag erklären könnten.

Der eigene Lauf liefert für Block 1, 64, 333 und die wechselnde Folge 1/7/64/333 jeweils:

```text
maxRatio=4.68323135375977
maxDb=13.4109122531407
maxAt=127
jump=0.0287752151489258
finalRatio=1
REGRESSION_ORACLE neutral_reenable_peak_le_1=EXPECTED_FAIL
```

`maxAt=127` ist das 128. Sample nach der neuen Publikation. `jump` ist die **absolute float-Sampledifferenz bei Eingang 0,5**; normiert auf den Eingang ist sie 0,0575504303. Diese beiden Bezugsgrößen dürfen nicht gegeneinander als abweichende Messung gelesen werden. Ein zusätzliches `beendeAudiohistorie()` im ausgeschalteten Zustand reproduziert dasselbe Maximum. Die Kontrolle 0 dB → aus → 0 dB → an bleibt exakt 1 mit Sprung 0. Die Gegenrichtung −24 → 0 hat kein positives Überschwingen; das ist kein Gegenbeweis gegen den +24-Fall.

Die Ursache bleibt `DspKern.cpp:374-394` (Rampen beim ENDE erhalten), `:418-424` (neues Ziel ab altem Istwert) und `DspKern.h:317-333` (Rampe), angewandt bei `DspKern.cpp:715-730`. Es wird kein Filterzustand aus einer fremden Bank benötigt. M-03 (`SONDE-015.md:1036`) und die Maximumwache in E-31 (`:3425`) sind der passende Bezug; M-07s Filter-/Hüllkurvenreset allein ist kein Gegenargument.

**Erreichbarkeit und Testgrenze.** Die Hostparameter sind über `SondeProcessor.cpp:1296-1303`, `:1413-1443` und `NakamaTransaktion.cpp:340-347` an den Publikationsweg angeschlossen. Der Nachweis führt jedoch den Kern direkt aus; er misst nicht die Hostereignissammlung oder deren Wartezeiten. Eine Hostreproduktion muss die jeweiligen Zustände lange genug wirksam werden lassen. Der behauptete Audioausschlag selbst ist jetzt eine echte Komponentenausführung statt einer Kopie seiner Rechenformel.

**Orakelprüfung.** `peak <= 1 + 1e-6` folgt hier aus dem neutralen Zielprogramm und dem unveränderten Dry-Signal; die Schranke wird nicht aus dem fehlerhaften Rampenablauf berechnet. Im Auditcaller ist dieses zukünftige Produktkriterium absichtlich nur als `EXPECTED_FAIL` ausgegeben, während `require(peak > 4.68 && Endwert == 1)` die Reproduktion sichert. Als dauerhafter Regressionstest muss die Erfolgsbedingung umgedreht werden und alle Samples auf Endlichkeit sowie absoluten Betrag prüfen. Exit 0 bedeutet hier ausdrücklich **Fehler reproduziert**, nicht Fehler behoben.

Die neuen Messwerte gehören als Beweisvertiefung unter T3-15-05. Die kleinste Reparatur muss Pfadrampen bei Ruhe beziehungsweise beim Eintritt ins neue Programm zum aktuellen Ziel kohärent initialisieren, ohne das Ausblenden des alten Programms oder den normalen Rampenweg zu brechen. Neben Output-Trim die benachbarten globalen Rampen gezielt prüfen; der ausgeführte +13,41-dB-Nachweis darf nicht ungeprüft als Zahlenbeleg für jede dieser Rampen ausgegeben werden.

## T3-14-02 und T3-15-08: keine gegensätzlichen Pauschalreparaturen

**T3-14-02, neue stärkere Eintrittsprüfung.** `dsp_probe.cpp:90` baut den echten `Transaktionskern` mit `DspKernAusfuehrung`. Nach einer Sekunde dynamischem Bell 1 kHz, Q 2, Range −12 dB, Threshold −40 dB, Attack 0,1 ms, Hold 500 ms, Release 5000 ms erfolgen ein echtes Remove und ein echtes Belegen von Slot 0, beide mit passender Basisrevision. Dazwischen läuft kein Audio. Die Produktergebnisse werden auf Commit geprüft. Danach läuft nur Stille.

```text
TRANSACTION_HISTORY removeRevision=1 addRevision=2 slot=0
dynamicDb=-12 audioPeak=0.102507740259171
```

Die frisch belegte Kontrollinstanz startet dagegen bei Auslenkung 0 und stillem Ausgang. `NakamaTransaktion.cpp:525-549` setzt die Belegung tatsächlich zurück und wieder neu; die Ursache liegt hinter dem erfolgreichen Commit. `DspBankPool.cpp:123-148` darf eine noch nicht vom Audio übernommene Publikation verdrängen. `DspProgramm.cpp:139-158` vergleicht nur die schließlich sichtbare Topologie; bei deren Gleichheit kopiert `DspKern.cpp:397-407` die alte Historie. M-121 (`SONDE-015.md:1213`) nennt ausdrücklich den neu belegten Slot, der den Wert seines Vorgängers nicht erben darf. Der stärkste Einwand „der Aufrufer simuliert nur occupied=false/true“ ist mit der zusätzlich ausgeführten echten Transaktionsfolge widerlegt. Ein heutiger Belegen-/Remove-UI-Handgriff bleibt nicht nachgewiesen; die Beweisstufe ist der gebaute API-Vertrag.

**T3-15-08, legitime unveränderte Historie.** Ein eingeschwungener Slot 0 mit Bell 50 Hz, Q 8, +12 dB verarbeitet einen 50-Hz-Sinus. Slot 5 ist bereits belegt, neutral und ausgeschaltet; nur sein `enabled` wechselt auf true (`dsp_probe.cpp:92-94`). Ein zweiter, gleich eingeschwungener Originalkern ohne diesen Wechsel ist die Audio-Referenz. Die erwartete Neutralität des Zusatzbands stammt aus gain=0, nicht aus einer nachgeschriebenen Filtertransiente. Der eigene Lauf misst über ganze 50-Hz-Perioden **−8,002865 dB** im ersten Perioden-RMS und einen Verlauf erst nach **200 ms** durchgehend innerhalb 1 dB. Die ältere −6,57-dB-/220-ms-Angabe benutzt Periodenspitzen statt Perioden-RMS; das ist eine andere Messgröße, kein still zu überschreibender Altwert.

`DspProgramm.cpp:148-156` macht wegen Slot 5 den gesamten Übergang inkompatibel, wodurch `DspKern.cpp:397-407` auch Slot 0 nicht übernimmt. Zusätzlich zeigt der Dynamik-Gegenfall: Nur Output-Trim ändern erhält −12 dB Auslenkung, derselbe finale Trim mit neutralem Zusatzslot setzt sie auf 0. E-8 (`SONDE-015.md:3400`) entscheidet den Kaltstart ausdrücklich. Daher wird der vorhandene Lückenstatus nicht zu einem Regelverstoß hochgestuft; die Zusage zur unveränderten Slot-Historie fehlt weiterhin. E-19 (`:3412-3413`) erlaubt bereits die Zustandsübernahme bei normalen Rampen.

**Gemeinsamer kleiner Korrekturumfang.** Ein neuer Slot-Lebenszyklus muss die alte Historie verlieren, auch wenn die Zwischenpublikation verdrängt wird. Derselbe unveränderte Slot-Lebenszyklus soll nach einer ausdrücklich festgelegten Kompatibilitätsregel seine Historie behalten können, auch wenn ein anderer Slot wechselt. Typ-, Modus- oder Detektorwechsel am betroffenen Slot benötigen eine eigene Kompatibilitätsentscheidung. Weder „immer alles nullen“ noch „immer alles kopieren“ erfüllt beide Fälle. Die drei Pflichtregressionen gehören zusammen: echte Remove-/Belegen-Commits ohne Audio dazwischen; unveränderter tiefer warmer Slot bei neutralem Wechsel in anderem Slot; gewöhnliche kontinuierliche Änderung mit legitimer Historienübernahme.

Der zusätzliche echte Same-Instance-Ladestart (`dsp_probe.cpp:91`) bewahrt bei gleicher Topologie Audio und Tap bitgenau. Ein neu vorbereiteter kalter Kern ist deshalb kein allgemeines Orakel für jeden Save-/Loadpfad. F12 bleibt für den noch zu definierenden Warm-/Kaltvergleich eine Produktentscheidung; daraus entsteht kein neuer Datenverlustbefund.

## Abschlussgrenze

Die neuen zentralen Schlussfolgerungen sind ein enger D-01-Lückenkandidat, die begründete Nichtaufnahme von D-02 und neue aktuelle Ausführungsbelege unter bestehenden IDs. Nicht ausgeführt wurden ein echter Mono-VST3-/FL-Lauf, ein Hostlauf bei 32 kHz, eine komplette Hostautomationsfolge für T3-15-05 oder der gesamte Kanon. Die übrigen D-Felder wurden durch diesen Auftrag nicht nochmals vollständig unabhängig validiert. Produkt-/Audiobereitschaft folgt daraus nicht.
