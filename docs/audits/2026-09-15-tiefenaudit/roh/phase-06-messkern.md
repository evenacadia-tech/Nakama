URTEIL: DEFEKTE 2 · LÜCKEN 1 · HÄRTUNGEN 0 — Der Messkern hat zwei zusätzliche Brüche bei Randwerten und Gültigkeit sowie eine unbestimmte Nachweisgrenze im statistischen Experimentpfad.

# Tiefenaudit 3 — Phase 06: Messkern und Numerik

## Bindung des Laufs

- Auftrag: Dirigent, 15.09.2026, 23:28 Uhr; Kurzname `messkern`.
- Branch: `master`; gebundener HEAD: `397067060d47074e67d1417ced8063fce8d5025d`.
- Beginn: 15.09.2026, 23:28:28 MESZ / 21:28:28 UTC.
- Ende und Dauer: siehe Abschlussvermerk unter „Scope-Beweis“.
- Prüfer: Codex. Die genaue Modellkennung und der Effort sind laut Auftrag im externen Startlog gebunden; diese Metadaten waren hier nicht unabhängig auslesbar. Es wird keine Kennung aus einem Vorgängerlauf übernommen.
- Vorgehen: Quellenlektüre, vollständige Inventur des Kernmusters, Aufrufverfolgung, analytische Herleitung und eigener Skeptikerpass. Keine Unteragenten. Die lokale Skill-Anleitung `.agents/skills/nakama-plugin-quality/SKILL.md` wurde für die Belegdisziplin gelesen; das ausdrückliche Bau- und Testverbot dieses Auftrags bestimmt den Beweisweg.
- Nur lesende Git-/Such-/Dateikommandos und Python für Text/AST/JSON/WAV-Header sowie elementare Algebra. Kein Produktcode wurde ausgeführt; kein Build, Test, Kanon, Rotlauf, Installer oder Hostlauf wurde gestartet. Nur dieser Bericht und der angehängte Phase-06-Abschnitt sind Schreibziele.
- Fremder Ausgangsstand: `.claude/settings.local.json`, `briefing-hub/`, `nimbalyst-local/` und `docs/audits/2026-09-15-tiefenaudit/roh/phase-06-auftrag.txt` waren untracked und bleiben unberührt.
- Manifeste, Register, Entwurf und frühere Audits dienen ausschließlich als Zusage-, Gegenbeleg- und Dublettenquelle. Sie sind keine Befundfläche.

## Befundtabelle

| ID | Schwere | Klasse | Kategorie | Ort | Befund | Beleg | Bricht | Vorschlag | Status |
|---|---|---|---|---|---|---|---|---|---|
| T3-06-01 | medium | K3 | DEFEKT | eq-copilot/plugin/core/analysis/featureengine/Spektrum.h:426; eq-copilot/plugin/core/analysis/featureengine/Spektrum.h:449; eq-copilot/plugin/core/analysis/featureengine/Lautheit.h:89; eq-copilot/plugin/src/AnalyseEngine.cpp:138 | bekannt NAK-283: Zusätzlich zum geschlossenen F06-Fehler werden DC und Nyquist in der FeatureEngine-PSD doppelt gewichtet. Bei stationärem DC mit −61 dBFS liefert das Zeitgate Aktivität 0, das Spektralgate wegen +2,21849 dB Fehler Abdeckung 1; die andere AnalyseEngine halbiert beide Randbins korrekt. | Quellenherleitung für das periodische Hann aus FeatureEngine.h:1118: Σw²=3N/8, X[0]=AN/2, X[1]=−AN/4. Der Faktor 2 auch am Rand ergibt nach Δf-Integration 5A²/3 statt A²; 10·log10(5/3)=2,218487496. DC bzw. alternierende ±A-Samples sind erreichbare endliche Eingaben. FFT-Parseval-Golden: AnalysisGoldenTestMain.cpp:981; die gekoppelten Aktivgate-Fälle in Sonde013EventWireTest.cpp:2058 verwenden Sinus innerhalb des Spektrums. | „Sie liefert kohärente Mess-Snapshots; der Editor hält Anzeigezustand und rendert nur neue Snapshot-Revisionen.“ (CLAUDE.md:186; NAK-283 M-46 nennt dieselbe Invariante.) | DC und Nyquist vor jeder PSD-Nutzung halb gewichten und die Gate-Kopplung für DC/Nyquist mit analytisch bekannter Energie prüfen. | offen |
| T3-06-02 | medium | K4 | DEFEKT | eq-copilot/plugin/src/AnalyseEngine.cpp:352; eq-copilot/plugin/src/AnalyseEngine.cpp:387; eq-copilot/plugin/sonde/SondeProcessor.cpp:638; eq-copilot/plugin/core/analysis/featureengine/Frame.h:590; eq-copilot/plugin/core/ipc/TelemetryClient.cpp:163 | bekannt NAK-283: Über den zusätzlichen LUFS-I-Zulieferweg gelangen weiterhin Messwerte aus ersetzten Samples in einen verriegelten Frame. Die Sonde setzt LUFS-I und Unsicherheit nach der FeatureEngine-Sperre erneut; im integrierten Histogramm bleiben die aus Ersatznullen gebildeten Zellen ohne Gültigkeitsmerkmal erhalten. | Lesende Aufrufkette: NaN/Inf → Ersatznull in AnalyseEngine::verarbeite → endliche K-Energie an loudness.zelle → lautheitFuerTelemetrie prüft nur endliches Wertepaar → SondeProcessor setzt die Präsenzbits trotz nichtEndlichRahmen>0 → P2 serialisiert das Paar. Nach gefüllter Lautheitshistorie reicht ein NaN in einem publizierten Rahmen; kein Transportwechsel ist nötig. Vertrag.h:620 beschreibt diese Eingangssample-Sperre ausdrücklich. | „Bei den Rahmenskalaren fehlt statt dessen das Präsenzbit. Der Zähler wird ausgewertet, nicht ignoriert.“ (SONDE-013 M-07, docs/beweise/SONDE-013.md:476). | Die Ungültigkeit bis zu LUFS-I und dessen überlappenden Integrationsblöcken mitführen und das zugemischte Paar einschließlich PLR nur mit gültigem Bezug freigeben. | offen |
| T3-06-03 | medium | K3 | LÜCKE | broker/src/coordinator/experiment.rs:585; broker/src/coordinator/experiment.rs:1606; broker/src/coordinator/experiment.rs:417; broker/src/coordinator/experiment.rs:493; broker/src/coordinator/experiment_verdrahtung.rs:974 | 400 Bootstrapziehungen setzen den kleinsten zweiseitigen p-Wert auf 2/401. Mit FDR q=0,05 bleiben bei m=221 gültigen Bändern selbst beliebig stabile Änderungen in nur 1 bis 22 Bändern grundsätzlich unsignifikant; bei m=64 sind mindestens 7 erforderlich. Ein vorhandenes Intervall ohne Null wird dann als Unveraendert ausgegeben. Eine Regel zur notwendigen Auflösung im Verhältnis zur getesteten Bandfamilie fehlt. | Rein algebraisch: k≥ceil(m·(2/401)/0,05). Konstant von null verschiedene Zeitreihen erreichen p_min, konstante Nullreihen p=1; mehr Fenster oder größere Deltas senken p_min nicht. Die gesamte gültige Bandfamilie wird geprüft. Die isolierten FDR-Goldens (Sonde013ExperimentGoldenTest.cpp:217; broker/tests/sonde013_experiment.rs:506) speisen 20 Werte 0,0001 ein, die der produktive 400er-Bootstrap nicht erzeugen kann. | keine Zusage | Bootstrapauflösung und getestete Bandfamilie aufeinander abstimmen und die gesamte Kette für schmale Änderungen sowie den Ausgang an der Auflösungsgrenze festlegen und prüfen. | offen |

Zähler: **critical 0 · high 0 · medium 3 · low 0**; **2 Defekte · 1 Lücke · 0 Härtungen**; **16 widerlegte beziehungsweise als bereits bekannt ausgeschiedene Verdachte**; **2 Produktfragen**.

## Herleitung und Skeptikerpass der überlebenden Befunde

### T3-06-01 — Die beiden selbstkonjugierten FFT-Bins tragen den Faktor 2 zu viel

**Kette und Einheit.** `FeatureEngine::rechneFenster` berechnet eine einseitige Leistungsdichte in Amplitude²/Hz. Das Kanalenergiemittel entsteht aus den FFTs von M=(L+R)/2 und S=(L−R)/2. `Spektrum.h:426` setzt für sämtliche N/2+1 Bins dieselbe Normierung 2/(fs·Σw²), einschließlich DC und Nyquist. `Spektrum.h:449` integriert anschließend über Δf. Bei einer einseitigen Dichte haben nur die inneren Bins einen Partner auf der negativen Frequenzachse. Die zweite Implementierung in `src/AnalyseEngine.cpp:138` berücksichtigt dies durch Halbieren der beiden Randbins.

**Lesender Reproduktionsweg.** Eine vorbereitete FeatureEngine erhält bei 48 kHz fortlaufende 512er-Blöcke mit L=R=A über mindestens drei Sekunden, ohne Transportwechsel und ohne Passage. A=10^(−61/20) ist endlich und weit im zulässigen Floatbereich. Stattdessen funktioniert auch L=R=A·(−1)^n für den Nyquist-Rand. Keine manipulierte Repoquelle, kein Testhaken, keine extreme Zahl wird vorausgesetzt.

Für das tatsächlich verwendete periodische Hann (`FeatureEngine.h:1118`) gilt im DC-Fall:

- Σw² = 3N/8.
- X[0] = AN/2; X[1] = −AN/4; die übrigen nichtnegativen Bins sind null bis auf numerische Rundung.
- Richtige integrierte Leistung: (A²N²/4 + 2·A²N²/16)/(N·3N/8) = A².
- Heutige Rechnung: 2·(A²N²/4 + A²N²/16)/(N·3N/8) = 5A²/3.
- Fehler: 10·log10(5/3) = **+2,218487496 dB**.
- Zeitgate (`Lautheit.h:89`): −61 dB, inaktiv. Spektralgate: **−58,781512504 dB**, aktiv. Nach dem Anlauf stehen deshalb `aktivitaet=0` und `abdeckung=1`.

Der Effekt betrifft die tatsächliche Rechenstelle und beide FFT-Stufen. Die Bandkappe bei Nyquist beseitigt ihn nicht: Die Aktiventscheidung summiert zuvor ausdrücklich alle Bins. Dass DC außerhalb der sichtbaren Nutzbänder liegt, ändert die Abdeckungszählung ebenfalls nicht.

**Gebrochene Zusage.** Die Kohärenz der Mess-Snapshots aus `CLAUDE.md:186` ist der Invariantenanker. `featureengine/Vertrag.h:716` präzisiert den gemeinsamen Energiebezug von Aktivität und Abdeckung. NAK-283 M-46 benutzt ausdrücklich denselben Invariantenanker. Dessen vorhandene beiden Sinusfälle werden hier nicht fälschlich als fehlschlagend behauptet: Der neue Fall liegt am Rand der FFT, außerhalb dieser Eingaben.

**Skeptiker.** Der fehlende Δf-Faktor aus dem alten F06 ist repariert (`Spektrum.h:448`); dieser Bericht öffnet ihn nicht erneut. Das FFT-Golden besitzt eine korrekte Parseval-Gewichtung und einen DC-Fall (`AnalysisGoldenTestMain.cpp:957`, besonders `:981` und `:991`), prüft dort aber die FFT, nicht die PSD-Normierung samt FeatureEngine-Gates. Die F06-Fälle in `Sonde013EventWireTest.cpp:2058` verwenden innere Sinusbins und Stille. Eine Quelle, die das verdoppelte Randgewicht als beabsichtigte Messdefinition festlegt, wurde nicht gefunden.

**Kleinster Beweis für die spätere Reparatur.** Ein konstant endliches Signal und ein alternierendes Signal knapp unter dem Aktivgate müssen in beiden Stufen ihre analytische Energie behalten; der Test muss den gemeinsamen Frame auswerten. Die PSD-Korrektur gehört vor Band-, Fluss- und Gate-Nutzung. Kein Laufzeitfehler wurde in dieser Session gemessen.

### T3-06-02 — Das zugemischte integrierte Loudness-Paar umgeht die NaN-Sperre

**Die vollständige Erzeugerkette.**

1. `src/AnalyseEngine.cpp:352` und `:357` ersetzen nicht-endliche Eingangssamples durch Null und zählen sie.
2. Dieselben Ersatznullen durchlaufen die K-Filter. `:387` übergibt ihre endliche Zellenergie an `LoudnessAccumulator::zelle`. Ein Merkmal „diese Energie enthält ersetztes Eingangsmaterial“ wird nicht mitgegeben.
3. `AnalyseEngine::lautheitFuerTelemetrie` (`:710`) akzeptiert einen endlichen integrierten Wert plus endliche Unsicherheit. Der Eingangssample-Zähler ist keine Freigabebedingung.
4. Parallel sperrt `FeatureEngine::fuelleSkalare` den belasteten Frame (`Frame.h:590`). Laut `Vertrag.h:620` dürfen sampleabhängige Rahmenskalare bei `nichtEndlichRahmen>0` kein Präsenzbit tragen.
5. `SondeProcessor.cpp:638` kopiert diesen Frame und setzt anschließend bei einem endlichen Loudness-Paar beide LUFS-I-Präsenzbits wieder (`:642`, `:644`). Es fehlt die Prüfung auf die gezählte Ungültigkeit.
6. `TelemetryClient.cpp:66` prüft Paarigkeit und Endlichkeit; `:163` schreibt beide Werte in den P2-Frame. Der relevante Eingangszähler wird dort nicht als alternative Sperre ausgewertet.
7. Der Leser übernimmt ein endliches Paar (`SourcesModel.cpp:1571`), das die bestehende Anzeige als integrierte Lautheit mit Unsicherheit nutzt (`PluginEditor.cpp:1297`). Die Anzeige wurde nur als Messwertverbraucher gelesen; Gestaltung ist nicht Prüfgegenstand.

**Erreichbarer Sonderpfad.** Nach beispielsweise acht Sekunden eines konstanten, deutlich über dem Loudness-Gate liegenden 1-kHz-Stereosignals wird ein einzelnes linkes Sample in einem später publizierten Rahmen NaN. Die Session und Zeitachse bleiben unverändert. Die integrierte Historie und ihre Unsicherheit bleiben endlich, während die FeatureEngine den betroffenen Frame korrekt sperrt. Das zugemischte Paar kann ihn trotzdem verlassen. Nach einem sauberen Folgeframe bleibt darüber hinaus die zuvor aus Ersatznullen gebildete Energie im integrierten Bestand. PLR kann später ebenfalls auf diesem Bezug aufbauen, sobald sein True-Peak-Bezug wieder freigegeben ist.

Diese Herleitung behauptet keine große LU-Abweichung durch ein einzelnes Sample. Der Bruch ist die positive Messbehauptung trotz gezählter ungültiger Messgrundlage; wiederholte ungültige Samples vergrößern den numerischen Anteil.

**Skeptiker.**

- `LoudnessGoldenTestMain.cpp:458` injiziert nicht-endliche **Energiezellen** in den Akkumulator. Eine aus NaN-PCM bereits zu einer endlichen Zahl sanitierte K-Energie erreicht diese Wache nicht.
- `Sonde013DynamicsTest.cpp:625` prüft die FeatureEngine-Metriken. Der später in der Sonde zugemischte LUFS-I-Pfad gehört nicht zu diesen neun geprüften Trägern.
- `Sonde012LoudnessSourceTest.cpp` prüft unter anderem Paarigkeit, Sammeln und Stille, aber nicht die vollständige PCM-NaN→K-Zelle→Sonde-Kette.
- `GoldenTestMain.cpp:725` verlangt eine endliche Erholung nach einem NaN-Abschnitt und anschließend sauberem Material. Das gestattet keine LUFS-I-Freigabe im belasteten Frame und prüft nicht die Gültigkeit der integrierten Fenster.
- NAK-283 M-60/M-76 betreffen konkrete Fenster und Historien von M/S-Lautheit, Peak und Headroom. Sie sind kein vorhandener Gegenbeweis für diesen zusätzlichen I-Zulieferweg. NAK-295s bekannte Anlauf-/Nachlaufgrenzen ersetzen die fehlende Information ebenfalls nicht.

**Kleinster Fixbereich.** Die Erzeuger müssen die Ungültigkeit bis zum integrierten Akkumulator mitführen und sämtliche 400-ms-Blöcke behandeln, die eine ungültige Zelle überlappen. Das Mischen in der Sonde darf den verriegelten Frame nicht wieder freigeben. Ein pauschales dauerhaftes Vergiften der gesamten Session wäre keine geeignete Folgerung: vorhandene saubere Historie und spätere Erholung müssen erhalten bleiben. Die spätere Wache muss durch den tatsächlichen Sonden-Erzeuger bis zum serialisierten Paar reichen.

### T3-06-03 — Die endliche Bootstrapauflösung passt nicht zu jeder FDR-Familie

**Keine behauptete Produktzusage.** Der Code legt eine feste Zahl von 400 Bootstrapziehungen fest, aber keine minimale erfassbare Bandbreite einer Änderung. Deshalb wird dieser Punkt als **LÜCKE** und nicht als Defekt eingeordnet.

Für eine Zeitreihe, deren Bootstrapmittel alle strikt dasselbe von null verschiedene Vorzeichen haben, ist der kleinste p-Wert durch `experiment.rs:1606` fest:

`p_min = 2 / (400+1) = 0,004987531172...`

Die Benjamini-Hochberg-Korrektur in `:1675` verlangt für mindestens einen angenommenen Rang k:

`p_(k) ≤ 0,05 · k/m`

m ist die Zahl aller in dieser Auswertung gültigen Bänder mit Zeitreihe. Daraus folgt:

| Gültige Bänder m | Mindestens gleichzeitig am p-Minimum liegende Bänder k |
|---:|---:|
| 11 | 2 |
| 64 | 7 |
| 100 | 10 |
| 150 | 15 |
| 221 | 23 |

Ein Quellfall ohne Zufallsunsicherheit ist unmittelbar formulierbar: k Bänder erhalten in jedem Fenster dasselbe endliche Delta, alle anderen gültigen Bänder überall Delta 0. Dann stehen k p-Werte bei p_min und die übrigen bei 1. Unterhalb der gezeigten Mindestzahl gibt es keinerlei signifikantes Band. Eine größere Zahl sauberer Fenster und ein größerer Effekt können diese Schranke nicht verbessern. Die Tabelle ist eine bedingte Aussage über m; es wird **nicht** behauptet, dass jede reale Sonde bei jeder Samplerate alle 221 Bänder gültig liefert.

**Erreichbarkeit und Folge.** `experiment_verdrahtung.rs:974` bildet die vollständige gültige Bandfamilie und ihre Zeitreihen; sie wird nicht für den FDR-Schritt auf das Zielband reduziert. `experiment.rs:393` führt Bootstrap und FDR zusammen. `:493` ergibt bei einem vorhandenen Intervall und null signifikanten Bändern `Unveraendert`, selbst wenn das Intervall null nicht enthält. Clusterbildung nach der Signifikanzentscheidung kann einen bereits überall negativen Entscheid nicht retten. Der Befund setzt gültige numerische Evidenz voraus, keine sabotierte Fixture und kein NaN.

**Skeptiker und Golden-Lücke.** Die C++- und Rust-FDR-Goldens verwenden 20 p-Werte von 0,0001 (`Sonde013ExperimentGoldenTest.cpp:217`, `broker/tests/sonde013_experiment.rs:506`). Das ist ein geeigneter isolierter Test der FDR-Funktion. Es ist jedoch kein Beweis, dass die produktive 400er-Bootstrapquelle solche Werte erzeugen und schmale Änderungen erkennen kann. Die Funktion ist für m=0 abgesichert; auch dieser Rand behebt die Auflösungsfrage nicht. NAK-212s bekannte Richtungs-/Bootstrapthematik ist ein anderer Punkt und wird hier nicht erneut gezählt.

**Fehlende Regel.** Festzulegen ist, welche schmalen Änderungen die Auswertung in welcher gültigen Bandfamilie überhaupt auflösen können muss und welcher Ergebniszustand gilt, wenn die eingestellte Auflösung das prinzipiell verhindert. Darauf aufbauend können Zahl der Ziehungen beziehungsweise Verfahren angepasst werden. Allein 400 durch eine große Konstante zu ersetzen wäre ohne Kosten- und Regelprüfung keine begründete Reparatur.

## Einheiten, Fenster und Messwertverbraucher

Die folgende Zuordnung bezieht sich auf die gesamte unten inventarisierte Kernpopulation. Einheiten und Präsenzbits wurden bis zu den numerischen Schreibern und Lesern verfolgt; die UI-Lektüre beschränkt sich auf die bestehenden Zahlenverbraucher.

| Größe | Rechenbezug und Zeitfenster | Übergabe / verbleibende Grenze |
|---|---|---|
| Spektrum / LTAS / Bandpegel | Kanalenergiemittel, periodisches Hann, Dichte in Amplitude²/Hz, Bandmittel aus Bin-Dichten; Hauptstufe 4096, Bassstufe 16384 Samples bei je 50 % Hop; fs-abhängiges Δf | `Spektrum.h:412`; Gitterzuordnung und Nyquist-Kappe in `Zeit.h:120`. Quantisierung und Bitmap bleiben separate Größen. T3-06-01 betrifft die Randgewichte vor dem Bandmittel. |
| Live- und Evidenzquantisierung | Live dB in 0,1-Schritten, fokussierte Evidenz in 0,01-Schritten; Sättigung, Gültigkeitsbitmap und Encoding werden mitgeführt | `Vertrag.h:537`, `NakamaEvidenz.cpp`, `broker/src/coordinator/evidenz.rs:253`; das 0,01-Paarfixture enthält −3000/−2700 für −30/−27 dB. Kein zusätzlicher Faktor-10-Befund im gelesenen Pfad. |
| Aktivität / Abdeckung | Aktivität: Anteil aktiver 100-ms-Zellen; Abdeckung: Anteil aktiver vollständiger Hauptstufen-FFT-Fenster; beide Gate-Schwellen −60 dB | `Lautheit.h:89`, `Spektrum.h:448`, `Frame.h`. Die unterschiedlichen Zeitraster sind definiert; stationärer Stoff muss trotzdem den gleichen Energiebezug haben. |
| Sample-Peak / True Peak | Sample-Peak des Rahmens; True Peak mit FIR-Interpolation, Rahmen-, Kurzzeit- und Passagenbezüge getrennt | `TruePeak.h:171`, `Frame.h`. dBFS und dBTP sind nicht derselbe Peak. Der Passagenanfang begrenzt Support und Historie. |
| LUFS-M / LUFS-S | K-gewichtete Kanalenergiesumme; 4 bzw. 30 vollständige 100-ms-Zellen | `Lautheit.h:109`, `:132`, `:162`; ohne volles sauberes Fenster fehlt das Präsenzbit. |
| LUFS-I / Unsicherheit | Integrierte gegatete 400-ms-Blöcke mit 100-ms-Hop, fester Histogramspeicher; atomare Übertragung von LUFS-I und Unsicherheit in LU | `LoudnessAccumulator.h`, `AnalyseEngine.cpp:710`, `SondeProcessor.cpp:638`. T3-06-02 liegt zwischen Eingangsgültigkeit und diesem zusätzlichen Lieferweg. |
| PSR / PLR / Crest | PSR: True-Peak-Maximum desselben 3-s-Fensters minus LUFS-S. PLR: Passagen-True-Peak minus zugemischtes LUFS-I. Crest separat als Peak/RMS im Rahmen und im 3-s-Fenster | `Vertrag.h:634`, `Lautheit.h:201`, `:224`, `SondeProcessor.cpp:647`. Kein Sample-Peak als Ersatz für den definierten PSR-True-Peak. |
| LRA | Verteilung geeigneter Short-term-Werte, 1-s-Hop; mindestens 60 geeignete Werte, nicht bloß 60 s Wanduhr | `Lautheit.h:65`, `:255`. Stille und ungültige Fenster bauen keine scheinbare Reife auf. |
| Headroom-Verteilung | P10/P50/P95 der gemessenen Rahmen-True-Peaks in dBTP, nicht das negierte „noch freie“ positive dB-Budget | `Vertrag.h:670`: mit Passage alle geeigneten Rahmen über Histogramm, ohne Passage definierte letzten höchstens 64 Rahmen; mindestens vier, Zahl wird mitgeschickt. |
| Evidenz-Perzentile / Konvergenz | P10/P50/P95 je gültigem Band aus dem begrenzten Verlauf; Konvergenz aus der Übereinstimmung der Halbfenster-Mediane; tatsächliche Fensterzahl wird mitgeführt | `Frame.h`, `Vertrag.h:692`, numerische Leser in C++ und Rust. Ein leeres Band wird nicht als gemessene Null interpretiert. |
| Korrelation / Kohärenz / Phase | Korrelation [−1,1]; Kohärenz [0,1] mit ausreichender Welch-Historie; Phase in Radiant, nur mit gültiger Kohärenz; gemessene Fensterdauer und Freiheitsgrade | `Stereo.h:52`, `:174`; `Vertrag.h:447`. Mindestens acht geeignete Welch-Fenster; bekannte Bassgrenze NAK-177. Verzögerungsgoldens verwenden die Kanalreihenfolge als Vorzeichenbezug. |
| M/S-Pegel / Seitenanteil / Folddown | Spektrales M/S und zeitsamplebasierter Folddown sind getrennte Rechnungen. Verhältnisgrößen heben gemeinsame Normierungsfaktoren auf | `Stereo.h:52`, `:163`, `:174`. Der absolute Bezug der M/S-dB bleibt Produktfrage P-06-01; keine neue undokumentierte dBFS-Garantie wird erfunden. |
| Fingerprint / Vergleichbarkeit | Versionierter Vektor aus 32 Energiefächern, 12 Chromafächern und 32 verdichteten Onset-Stützstellen; Vergleich aus Teilähnlichkeiten | `Fingerprint.h:50`, `Vertrag.h:332`, `broker/src/telemetrie.rs:60`. Nullvektoren sind ausdrücklich fail-closed. |
| Bootstrap / FDR / Konfidenz / Ursachenklasse | Reproduzierbare Saat, Zeitblöcke statt Band-Samples; validierte Evidenz und Deckelung vor starker Klasse; Hypothesenrevisionen werden geprüft | `experiment.rs:1480`, `hypothese/determinismus.rs`, `hypothese/rechnung.rs`, `hypothese_verdrahtung.rs:484`. T3-06-03 betrifft die Auflösung beim Zusammenschalten zweier für sich abgesicherter Schritte. |
| DSP-Detektor / Tap | Samplegetakteter Detektor mit ms→Sample-Bezug; Mess-Tap vor/nach Verarbeitung und gültige Samplezahl getrennt | `dsp/DspKern.cpp:427`, `dsp/DspFilter.h:224`; Goldenbereiche F/K prüfen Zeitkonstanten beziehungsweise endliche Grenzwerte. DSP wurde nur an dieser Messkante beurteilt. |

## Zeit, Lücken, Rücksetzung und Determinismus

- **Vollständige Grenzentscheidung:** `featureengine/Zeit.h:180` (`grenzeZwischen`), `:288` (`moeglicherStraddleIn`) und `:318` (`grenzeZiehen`) wurden samt allen 15 Funktionen der Datei gelesen. Epoche, Segment, lokale Samplekontinuität, Projektzeit und möglicher Loop-Straddle sind nicht durch die Paint- oder P2-Kadenz ersetzt. FFT-, Loudness-, Stereo-, Fingerprint- und Evidenzhistorien wurden bei dieser Rücksetzung gemeinsam verfolgt.
- **Passagen:** `FeatureEngine.h:749` / `:816` bilden Setzen/Löschen; `Zeit.h:50` / `:96` vereinigen beziehungsweise merken tatsächlich gemessenen Support. Das nachträgliche Binden einer Passage und ihr tatsächliches Startsample sind getrennt. Die NAK-283-M73/M74-Fälle in `Sonde013EventWireTest.cpp:2177` wurden als Gegenbelege gelesen.
- **Analyse-Drops:** Die Sondenfolge um `SondeProcessor.cpp:554`, `:614` und `:627` reicht bis zu beiden Analyseengines und deren Reset. Der Main-Pfad in `src/prozessor/Analyse.cpp` wurde vollständig gelesen. Publikationsverlust eines P2-Frames ist vom Verlust eines PCM-Analyseblocks zu unterscheiden; ein langsamer Empfänger ändert nicht automatisch das musikalische Messfenster. In dieser Phase wurde kein zusätzlicher zwingender Durchlauf eines FFT-/Loudnessfensters über eine erkannte Lücke hergeleitet.
- **Stereo-Blockgrößen:** B19 inventarisiert genau **18** Größen: 1, 2, 3, 7, 15, 16, 31, 64, 127, 128, 333, 512, 1024, 2048, 4096, 8192, 12345, 16384 (`Sonde013StereoGoldenTest.cpp:936`). Es prüft fünf Klassen plus Folddown. Das ist kein Sweep jeder Zahl von 1 bis 16384 und kein Sweep aller FeatureFrame-Felder.
- **Ereigniszeit:** Der Analyse-Golden enthält fünf Sampleraten und die fünf Blockgrößen 1, 333, 512, 2048, 16384 (`AnalysisGoldenTestMain.cpp:2317`). Er beurteilt die Samplezeit des stärksten Flussereignisses. Eine generelle blockunabhängige Callback-Verfügbarkeit aller Rahmenwerte folgt daraus nicht.
- **NaN-Fenster:** Die M-/S-Sperre hängt an allen Zellen des jeweiligen Fensters, nicht nur am aktuellen Frame. TP/Headroom nehmen belastete Rahmen nicht auf. Der zusätzlich zugemischte integrierte Pfad ist die verbliebene Ausnahme T3-06-02.
- **Nicht dynamisch bewiesen:** Scheduling-Reihenfolgen, gezielt erzwungene Audio→Analyse-Überlast, reale Seek-/Tempo-/Rate-Wechsel und Langzeit-Überläufe wurden nicht ausgeführt. Die Quellenprüfung kann keine statistische Ausfallwahrscheinlichkeit solcher Läufe liefern.

## Golden-Inventur und Grenzen der Zusagenabdeckung

**Neun von neun Dateien des vorgegebenen Musters** wurden inventarisiert. Acht wurden vollständig im ausführbaren Code gelesen; beim großen DSP-Golden wurden alle Messkanten und ihre Hilfsfunktionen gelesen, die restlichen DSP-Verhaltensbereiche nur inventarisiert. Testquelltext ist hier ein vorhandener Detektor, kein Ergebnis eines aktuellen Testlaufs.

| Golden-Datei unter `eq-copilot/plugin/tests/` | Gelesener Bezug | Grenze / unabhängige Referenz |
|---|---|---|
| `AnalysisGoldenTestMain.cpp` | FFT, Gitter, Grenzen, Support, Zonen und Ereigniszeit | Parseval und synthetische Frequenzen sind analytische Bezüge. FFT-DC ist kein Golden der FeatureEngine-PSD-Gewichtung. |
| `GoldenTestMain.cpp` | Bestehende Analyse, WAV-/JSON-Vergleich, NaN-Erholung | Vier gespeicherte Referenzsignale; Referenzerzeugung über den externen Analysepfad. Keine in dieser Session nachgerechneten WAV-Messwerte. |
| `LoudnessGoldenTestMain.cpp` | Histogramm gegen getrennte frühere Vektorrechnung, Gating, Speicher und Zahlenränder | Implementierungsvergleich ist für die Umstellung nützlich, ersetzt aber nicht den unabhängigen EBU-LUFS-I-Bezug; bekannt NAK-199. |
| `Sonde013TruePeakGoldenTest.cpp` | Fälle 15–19, Sampleraten und analytische Interpolationsschranke; transiente Fälle 20–23 | Transienten gegen im Test validierte 64-fach-Referenz, nicht gegen denselben produktiven Interpolator. Der Abgleich mit offiziellen Testdateien ist bekannt NAK-293. |
| `Sonde013StereoGoldenTest.cpp` | Mono/Antiphase, Delay/Phase, Welch-Präsenz, Grenzen, 18 Blockgrößen | Analytische Identitäten und reale Monosumme als Bezüge; keine absolute M/S-dBFS-Zusage. |
| `Sonde013FingerprintGoldenTest.cpp` | Skalierung, Version, Mindeststoff, Vektorbildung und Teilähnlichkeit | Identität mit degeneriertem Onset-Teil ist ausdrücklich fail-closed; kein Beweis für jeden stationären Materialvergleich. |
| `Sonde013PrePostGoldenTest.cpp` | PRE/POST-Bezug, Paarung, Differenzen und Randfälle | Nur tatsächlich gültige und vergleichbare Eingänge rechtfertigen Deltas; fremde beziehungsweise unvollständige Bezugssysteme dürfen keinen starken Befund tragen. |
| `Sonde013ExperimentGoldenTest.cpp` | Bootstrap, FDR, Cluster, leere/nicht-endliche Zahlenpfade | Die FDR-Referenzwerte 0,0001 sind unabhängig vorgegeben. Ihre Kombination mit der produktiven Bootstrapauflösung bleibt T3-06-03. |
| `DspGoldenTestMain.cpp` | Detektorzeit, Oversize-Aufteilung/Taps und endliche Grenzwerte | Messkanten mit samplebezogener Referenz; keine Aussage dieses Audits über sämtliche EQ-/DSP-Programme oder den Nulltest zur Laufzeit. |

**Zu den ausdrücklich genannten Zusagen.**

- Entwurf 0.5 §56 und §58: Die Grenz-/Support-Goldens und PRE/POST-Fälle sind vorhanden. Eine frische Bestätigung dieser Gates wäre ein eigener erlaubter Testlauf; sie wird hier nicht aus Manifeststatus abgeleitet.
- SONDE-013 M-01 bis M-07 sowie NAK-283 M-44 bis M-60/M-73/M-74/M-76: Die konkrete Bezugskette wurde mit `Sonde013DynamicsTest.cpp`, `Sonde013EventWireTest.cpp` und den Golden-Dateien abgeglichen. Die zwei neuen Defekte liegen gerade jenseits der vorhandenen Eingangsmuster beziehungsweise am nachträglichen I-Zulieferer.
- SONDE-014 und Entwurf §59: Die numerischen Hypothesenfunktionen und ihre Verdrahtung wurden gegen die gelesenen Matrixbereiche und das P5-Korpus gestellt. Starke Klassen beruhen auf zusätzlichen Vergleichbarkeits-, Abdeckungs-, Revisions- und Belegbedingungen; ein großer Einzelpegel allein führt im gelesenen Pfad nicht zwingend zu einer Kausalbehauptung.
- **Rotbeweise:** Die Matrixeinträge bezeichnen vorhandene historische Rotpfade. Kein solcher Lauf wurde wiederholt. Für die drei jetzt konkretisierten Fälle wurde kein passender vollständiger Golden-/Rotpfad gefunden. Das bedeutet nicht, dass sämtliche anderen Tests im Repository rotbewiesen sind; eine vollständige Prüfung aller historischen Rohlogs ist ausdrücklich nicht erfolgt.
- **Korpuspopulation:** P4 enthält 26 Fälle, P5 27 Sitzungen. Ihre technischen Objekte und die Messrechnungen der Erzeuger/Prüfer wurden gelesen. Deklarative Sollzahlen sind nicht mit einem frischen Durchlauf des Coordinators gleichgesetzt. Die bereits registrierten Grenzen NAK-200/201/203/294 werden nicht als neue Golden-Befunde ausgegeben.

## Widerlegte Verdachte

Die Liste zählt sowohl fachlich widerlegte Hypothesen als auch Punkte, die wegen eines belegten Vorbefunds **kein neuer Befund** sind. Bekannte Einschränkungen werden dadurch nicht als behoben bezeichnet.

1. **F06 sei unverändert offen:** widerlegt. `Spektrum.h:448` integriert mit dem stufeneigenen Δf; `Sonde013EventWireTest.cpp:2058` enthält die Rate-/Stufenfälle. Neu ist nur die davon unabhängige Randgewichtung T3-06-01.
2. **True Peak komme weiterhin ungeprüft aus Material vor dem Passagenanfang:** für den gelesenen normalen Passagenpfad widerlegt. Setzen/Löschen in `FeatureEngine.h:749` / `:816`, Support-/Grenzlogik in `Zeit.h:318` und die zusätzlichen Fälle in `Sonde013EventWireTest.cpp:2177` verhindern die pauschale Behauptung. Bekannte kurze Nachlaufgrenzen bleiben NAK-295.
3. **Headroom verwende auch bei markierter langer Passage nur 64 Rahmen:** widerlegt. `FeatureEngine.h:307` akkumuliert die Passage in ein festes Histogramm; `Vertrag.h:675` begrenzt nur den ausdrücklich beschlossenen Zustand ohne Passage auf 64. M-76 sperrt ersetzte Rahmen vor dem Einschub.
4. **Der erste saubere Rahmen nach NaN mache LUFS-M/S sofort wieder gültig:** widerlegt. `Lautheit.h:53` markiert die Zelle; `:162` prüft das gesamte Fenster; `Frame.h:578` beschreibt die zweite Sperrstufe. T3-06-02 betrifft stattdessen den später zugemischten I-Wert.
5. **N-22 prüfe wegen eines 48-kHz-Speiserstempels ausschließlich eine auf 48 kHz zurückgesetzte Engine:** widerlegt nach erneutem Lesen. `Sonde013StereoGoldenTest.cpp:845` bereitet ausdrücklich 48/96 kHz vor. `FeatureEngine.h:643` übernimmt einen Blockstempel nicht als erneute Konfiguration; die Rechenrate wird in `:451` gesetzt. Der Stempel allein beweist keinen Reset auf 48 kHz. Der zunächst geäußerte Verdacht wurde in dieser Session korrigiert und wird nicht zum Befund.
6. **Die absoluten M/S-Zahlen seien eine neu gefundene dBFS-Vertragsverletzung:** nicht als neuer Defekt haltbar. `Stereo.h:52` und `:174` arbeiten hier mit Spektralenergie; Verhältnisgrößen bleiben davon getrennt. Der absolute Bezug ist bereits im Vorgängeraudit `UEBERGABE-AN-CLAUDE.md:288` als nicht hochgestufter Punkt genannt. Kein belastbarer absoluter dBFS-Verbrauchervertrag gefunden; siehe P-06-01.
7. **Ein stationärer Fingerprint mit leerem Onset-Teil beweise einen Materialwechsel-Bug:** als neue Defektbehauptung widerlegt. `Fingerprint.h:101`, `Vertrag.h:393` und `broker/src/telemetrie.rs:87` behandeln degenerierte Teilvektoren explizit fail-closed. Der Vorgänger nennt das Muster bereits in `UEBERGABE-AN-CLAUDE.md:289`; siehe P-06-02.
8. **m=0 oder eine leere/nicht-endliche Bootstrapreihe produziere zwangsläufig NaN oder Signifikanz:** widerlegt. `experiment.rs:1573` gibt p=1 zurück; `:1678` liefert bei leerer Familie die leere Auswahl. Die C++-Gegenstücke in `core/analysis/Statistik.h` wurden vollständig gelesen. Der neue Punkt ist die endliche Auflösung bei gültigen nichtleeren Reihen.
9. **Ein großer einzelner Score reiche im Hypothesenpfad für eine starke Ursachenbehauptung:** für die gelesene Kette widerlegt. `hypothese/rechnung.rs`, `befund.rs`, `enthaltung.rs` und `hypothese_verdrahtung.rs:484` koppeln mehrere Evidenzbestandteile und prüfen Revision/Bezug. Keine zusätzliche SONDE-015-artige überstarke Behauptung zwingend hergeleitet.
10. **Floatüberlauf in der alten RMS-/FFT-Kante sei eine neue Phase-06-Entdeckung:** ausgeschieden als bereits in Phase 01 erfasste T3-01-06/T3-01-07. Die gelesenen Stellen in `src/AnalyseEngine.cpp` und `src/prozessor/Analyse.cpp` rechtfertigen keine doppelte ID. Der Kern-`Fft.h` rechnet seine Größen in double; daraus wird keine Entwarnung für die andere Kante abgeleitet.
11. **Fehlende offizielle Referenzen seien neu:** ausgeschieden, bekannt NAK-199 für unabhängige LUFS-I-Referenzen und NAK-293 für die offiziellen transienten TP-Dateien 20–23. `Sonde013TruePeakGoldenTest.cpp:545` vergleicht gegen eine separat validierte 64-fach-Referenz. Das ist kein Vergleich gegen denselben produktiven Interpolator und kein Ersatz für die noch offene offizielle Gegenmessung.
12. **Bekannte fehlende oder konservativ gesperrte Vergleichsgrößen seien neue numerische Defekte:** ausgeschieden. Register NAK-171/177/193/204 deckt die gelesenen Einschränkungen bei Loudness-/Peak-Guardrails, Bass-Welch und Side-Guardrail ab. `Stereo.h:174` und `experiment_verdrahtung.rs:1159` liefern dazu keine zusätzliche bisher unbekannte Verletzung.
13. **P4-/P5-Sollzahlen oder ein begrenzter Sweep bewiesen bereits vollständige Güte:** als Schlussfolgerung widerlegt. NAK-200/201/203/294 nennt die einschlägigen Evaluations-, Hilfsprüfer- und Korpusgrenzen. Die 26/27 Fälle wurden als ganze Population gelesen; keine dieser bekannten Grenzen wird neu nummeriert.
14. **Ein im Hörmarker-Dauerzähler fehlendes letztes Fade-Stück lasse zwangsläufig unverriegeltes markiertes Material entstehen:** die weitergehende Folgerung ist widerlegt. Der Markerpfad in `src/HoerMarkierung.h` endet mit Fade; `core/ipc/InterventionsRing.h:78` berechnet anschließend doppelte gemeldete Dauer plus 100 ms Nachlauf. Die gelesene kleine Dauerabweichung reicht nicht aus, um die behauptete Taint-Lücke zu begründen.
15. **Beliebige ungerade/nicht-zweierpotenzige FFT-Größen seien ein erreichbarer FeatureEngine-Fehler:** widerlegt für diesen Aufrufgraphen. `FeatureEngine.h` konfiguriert die beiden festen Größen 4096/16384; `Fft.h` hat keinen ungeprüften Hostparameter für N. Ein künstlicher Direktaufruf außerhalb dieser Population wird nicht als Produktdefekt ausgegeben.
16. **Der externe Analysepfad mache wegen seiner anders gefassten PSR-Zahl das gespeicherte PSR-Golden falsch:** widerlegt für die gelesenen Fixtures. `tools/eq-copilot/erzeuge_fixtures.py` exportiert diesen PSR-Wert nicht in die geprüfte Referenz. Eine gleichnamige, aber ungenutzte Diagnosegröße beweist keinen falschen Golden.

## Produktfragen

Diese Fragen sind nur notiert; während der technischen Auditphase wird keine Produktentscheidung eingeholt.

- **P-06-01 — Absoluter M/S-Bezug:** Welchen absoluten Referenzbezug sollen `midDb` und `sideDb` langfristig zusagen: normierte Leistung, Leistungsdichte oder nur die heute daraus verwendeten relativen Größen? Der rohe spektrale Bezug kann sich mit N ändern, während Seitenanteil und Verhältnisgrößen stabil bleiben. Bereits im Vorgänger benannt; kein zusätzlicher Befund ohne diese Zusage.
- **P-06-02 — Stationäres Material ohne Onsets:** Soll ein degenerierter Onset-Teil weiterhin den Gesamtvergleich konservativ sperren, oder soll der Ergebnisgrund gesondert „Onset-Vergleich nicht beurteilbar“ heißen? Die bestehende Nullvektorbehandlung ist beabsichtigt; aus ihr wird hier kein Materialwechsel als Tatsache abgeleitet.

## Nicht geprüft

- Kein Build, kein Testlauf, kein Golden-Vergleich, keine Mutation/Rotprobe, kein Kanon, keine Installation und kein FL-/EqCopShot-Lauf. Die angeführten Reproduktionswege sind Quellenherleitungen.
- Keine PCM-Neuberechnung, kein Abhören und kein spektraler Neuvergleich der WAV-Dateien. Alle neun WAV-Header wurden gelesen; die Sampledaten wurden nicht als Musik analysiert.
- Keine vollständige automatische Gleichheitsmatrix „alle Metriken × alle Blockgrößen 1…16384 × alle Sampleraten“. Keine Messung von Thread-Interleaving, realem Overload, Transportwechseln oder Langzeit-Soaks.
- Keine vollständige historische Rotlog-Prüfung. „Vorhandenes Golden“ bezeichnet gelesenen Testcode; ein historisches PASS wird nicht als aktuelles Prüfergebnis übernommen.
- Kein Audit von Dokumentprosa, Zeilenverweisen, Trefferzahlen, Manifesthygiene oder Registerpflege. Kein Design, keine Optik, keine Umbenennung eingefrorener Identitäten und keine Reaktivierung geparkter Studien.
- DSP außerhalb seiner Detektor-/Tap-/Messkante, übrige Broker-Lebenszyklen und allgemeine Installer-/Runner-Qualität bleiben außerhalb dieser Phase.
- Die exakte Modell-/Effortkennung des externen Startlogs und die dortige Maschinenkonfiguration wurden nicht unabhängig gelesen. Das begrenzt diese Metadaten, nicht den Zugriff auf die angeforderten Produktquellen.


## Scope-Beweis

### 1. Inventurverfahren und Dateizahlen

Die Dateipopulation wurde mit `rg --files` ermittelt; Definitionen und Aufrufe wurden mit `rg` und lesendem Python gezählt. C++-Signaturtreffer wurden gegen die Quellen bereinigt (keine lokalen Objektinitialisierungen als Funktionen); für Python wurde ausschließlich `ast.parse` benutzt. Kein Modul wurde importiert oder ausgeführt. Arrays zählen als ein Feld, Präsenzbits als eigene Felder; symbolgleiche Methoden an verschiedenen Definitionen zählen getrennt.

Die Bereichsprotokolle enthalten **98 unterschiedliche gelesene Pfade**: **18 Kernheader, 20 weitere Plugin-Produktdateien, 12 C++-Testdateien, 18 Brokerdateien einschließlich eines Tests, 18 Python-Werkzeuge, eine externe Python-Referenz und 11 Kontext-/Skilldateien**. Hinzu kommen **18 strukturell gelesene Fixturedateien** sowie die unten separat genannten gezielten Kontextabfragen. Dies ist keine Behauptung, dass jede der 98 Dateien vollständig gelesen wurde. Ausführbarer Code wurde teilweise ohne reine Kommentar-/Docstringblöcke ausgegeben; für Absicht und Vertrag relevante Kommentare wurden zusätzlich gezielt gelesen. Bei gekürzten Toolausgaben wurden die maßgeblichen Quellbereiche nachgelesen. Die folgenden Bereiche machen verbleibende Grenzen sichtbar.

### 2. Ganze Kernpopulation — 18 Dateien, 234 Funktionsdefinitionen

Alle Dateien unter `eq-copilot/plugin/core/analysis/**` wurden vollständig gelesen. Gezählt sind **234 benannte Funktionen mit Körper**, einschließlich Inlinezugriffen und lokalen Klassenmethoden; Lambdas sind Bestandteil der umgebenden Funktion und keine weitere benannte Definition. Hinzu kommen **52 deklarierte private FeatureEngine-Methoden**, deren Definitionen in den Teilheadern bereits mitzählen, **5 `= default`-Definitionen** und **2 `= delete`-Deklarationen**. Die generierten Bandgitterzahlen wurden ebenfalls vollständig gelesen.

| Datei | Zeilen | Definitionen mit Körper | Jede Definition, als Symbol und Datei:Zeile |
|---|---:|---:|---|
| `eq-copilot/plugin/core/analysis/BandGrid.h` | 259 | 16 | `evidenzKante` — `eq-copilot/plugin/core/analysis/BandGrid.h:49`; `evidenzMitte` — `eq-copilot/plugin/core/analysis/BandGrid.h:53`; `liveKante` — `eq-copilot/plugin/core/analysis/BandGrid.h:57`; `liveMitte` — `eq-copilot/plugin/core/analysis/BandGrid.h:61`; `liveVon` — `eq-copilot/plugin/core/analysis/BandGrid.h:66`; `liveBisExkl` — `eq-copilot/plugin/core/analysis/BandGrid.h:70`; `skalierungVon` — `eq-copilot/plugin/core/analysis/BandGrid.h:135`; `quantisiere16` — `eq-copilot/plugin/core/analysis/BandGrid.h:148`; `quantisiere16` — `eq-copilot/plugin/core/analysis/BandGrid.h:165`; `quantisiereF32` — `eq-copilot/plugin/core/analysis/BandGrid.h:176`; `bitmapBytes` — `eq-copilot/plugin/core/analysis/BandGrid.h:199`; `bitmapNullen` — `eq-copilot/plugin/core/analysis/BandGrid.h:204`; `bitmapSetze` — `eq-copilot/plugin/core/analysis/BandGrid.h:210`; `bitmapLies` — `eq-copilot/plugin/core/analysis/BandGrid.h:217`; `plausibel` — `eq-copilot/plugin/core/analysis/BandGrid.h:236`; `energieAlsDb` — `eq-copilot/plugin/core/analysis/BandGrid.h:252` |
| `eq-copilot/plugin/core/analysis/BandGridZahlen.h` | 203 | 0 | keine; Datentabelle |
| `eq-copilot/plugin/core/analysis/Blindvergleich.h` | 203 | 12 | `bindeReihenfolge` — `eq-copilot/plugin/core/analysis/Blindvergleich.h:66`; `reihenfolgeGebunden` — `eq-copilot/plugin/core/analysis/Blindvergleich.h:75`; `uebernimmVergleichspegel` — `eq-copilot/plugin/core/analysis/Blindvergleich.h:91`; `lautheitAbgeglichen` — `eq-copilot/plugin/core/analysis/Blindvergleich.h:100`; `gainDbEingefroren` — `eq-copilot/plugin/core/analysis/Blindvergleich.h:115`; `sperre` — `eq-copilot/plugin/core/analysis/Blindvergleich.h:124`; `urteile` — `eq-copilot/plugin/core/analysis/Blindvergleich.h:136`; `geurteilt` — `eq-copilot/plugin/core/analysis/Blindvergleich.h:145`; `holeUrteil` — `eq-copilot/plugin/core/analysis/Blindvergleich.h:146`; `aufgedeckteReihenfolge` — `eq-copilot/plugin/core/analysis/Blindvergleich.h:154`; `loeschen` — `eq-copilot/plugin/core/analysis/Blindvergleich.h:171`; `sperrWort` — `eq-copilot/plugin/core/analysis/Blindvergleich.h:191` |
| `eq-copilot/plugin/core/analysis/featureengine/Fingerprint.h` | 199 | 3 | `FeatureEngine::fingerprintSchritt` — `eq-copilot/plugin/core/analysis/featureengine/Fingerprint.h:50`; `FeatureEngine::fingerprintJetzt` — `eq-copilot/plugin/core/analysis/featureengine/Fingerprint.h:122`; `FeatureEngine::fingerprintLeeren` — `eq-copilot/plugin/core/analysis/featureengine/Fingerprint.h:187` |
| `eq-copilot/plugin/core/analysis/featureengine/Frame.h` | 738 | 11 | `FeatureEngine::baueFrame` — `eq-copilot/plugin/core/analysis/featureengine/Frame.h:34`; `FeatureEngine::evidenzLeeren` — `eq-copilot/plugin/core/analysis/featureengine/Frame.h:154`; `FeatureEngine::rahmenLeeren` — `eq-copilot/plugin/core/analysis/featureengine/Frame.h:174`; `FeatureEngine::baueStempel` — `eq-copilot/plugin/core/analysis/featureengine/Frame.h:213`; `FeatureEngine::fuelleLive` — `eq-copilot/plugin/core/analysis/featureengine/Frame.h:331`; `FeatureEngine::fuelleEvidenz` — `eq-copilot/plugin/core/analysis/featureengine/Frame.h:380`; `FeatureEngine::ringInZeitfolge` — `eq-copilot/plugin/core/analysis/featureengine/Frame.h:410`; `FeatureEngine::perzentil` — `eq-copilot/plugin/core/analysis/featureengine/Frame.h:425`; `FeatureEngine::fuelleVerteilung` — `eq-copilot/plugin/core/analysis/featureengine/Frame.h:443`; `FeatureEngine::fuelleAbdeckungUndKonvergenz` — `eq-copilot/plugin/core/analysis/featureengine/Frame.h:518`; `FeatureEngine::fuelleSkalare` — `eq-copilot/plugin/core/analysis/featureengine/Frame.h:565` |
| `eq-copilot/plugin/core/analysis/featureengine/Lautheit.h` | 316 | 11 | `FeatureEngine::zelleSchliessen` — `eq-copilot/plugin/core/analysis/featureengine/Lautheit.h:30`; `FeatureEngine::lraBin` — `eq-copilot/plugin/core/analysis/featureengine/Lautheit.h:98`; `FeatureEngine::kurzLufs` — `eq-copilot/plugin/core/analysis/featureengine/Lautheit.h:109`; `FeatureEngine::momentanLufs` — `eq-copilot/plugin/core/analysis/featureengine/Lautheit.h:132`; `FeatureEngine::zellenFensterSauber` — `eq-copilot/plugin/core/analysis/featureengine/Lautheit.h:162`; `FeatureEngine::kurzfensterNeuBeginnen` — `eq-copilot/plugin/core/analysis/featureengine/Lautheit.h:182`; `FeatureEngine::kurzTruePeak` — `eq-copilot/plugin/core/analysis/featureengine/Lautheit.h:201`; `FeatureEngine::crestKurz` — `eq-copilot/plugin/core/analysis/featureengine/Lautheit.h:224`; `FeatureEngine::lraLu` — `eq-copilot/plugin/core/analysis/featureengine/Lautheit.h:255`; `FeatureEngine::lraBinMitte` — `eq-copilot/plugin/core/analysis/featureengine/Lautheit.h:294`; `FeatureEngine::lraPerzentil` — `eq-copilot/plugin/core/analysis/featureengine/Lautheit.h:301` |
| `eq-copilot/plugin/core/analysis/featureengine/Spektrum.h` | 739 | 8 | `FeatureEngine::passagenanfangNochVorn` — `eq-copilot/plugin/core/analysis/featureengine/Spektrum.h:39`; `FeatureEngine::verarbeiteSamples` — `eq-copilot/plugin/core/analysis/featureengine/Spektrum.h:52`; `FeatureEngine::schiebeStufe` — `eq-copilot/plugin/core/analysis/featureengine/Spektrum.h:329`; `FeatureEngine::rechneFenster` — `eq-copilot/plugin/core/analysis/featureengine/Spektrum.h:412`; `FeatureEngine::summeBereich` — `eq-copilot/plugin/core/analysis/featureengine/Spektrum.h:562`; `FeatureEngine::flussSchritt` — `eq-copilot/plugin/core/analysis/featureengine/Spektrum.h:580`; `FeatureEngine::medianDerHistorie` — `eq-copilot/plugin/core/analysis/featureengine/Spektrum.h:716`; `FeatureEngine::ereignisAblegen` — `eq-copilot/plugin/core/analysis/featureengine/Spektrum.h:730` |
| `eq-copilot/plugin/core/analysis/featureengine/Stereo.h` | 318 | 4 | `FeatureEngine::stereoSchritt` — `eq-copilot/plugin/core/analysis/featureengine/Stereo.h:52`; `FeatureEngine::stereoSample` — `eq-copilot/plugin/core/analysis/featureengine/Stereo.h:163`; `FeatureEngine::stereoAuswerten` — `eq-copilot/plugin/core/analysis/featureengine/Stereo.h:174`; `FeatureEngine::stereoLeeren` — `eq-copilot/plugin/core/analysis/featureengine/Stereo.h:303` |
| `eq-copilot/plugin/core/analysis/featureengine/Vertrag.h` | 773 | 4 | `nak29Verstoss` — `eq-copilot/plugin/core/analysis/featureengine/Vertrag.h:192`; `frameversuch` — `eq-copilot/plugin/core/analysis/featureengine/Vertrag.h:255`; `fingerprintAehnlichkeit` — `eq-copilot/plugin/core/analysis/featureengine/Vertrag.h:379`; `leeren` — `eq-copilot/plugin/core/analysis/featureengine/Vertrag.h:547` |
| `eq-copilot/plugin/core/analysis/featureengine/Zeit.h` | 601 | 15 | `FeatureEngine::projektVorwaerts` — `eq-copilot/plugin/core/analysis/featureengine/Zeit.h:36`; `FeatureEngine::supportVereinen` — `eq-copilot/plugin/core/analysis/featureengine/Zeit.h:50`; `FeatureEngine::supportMerken` — `eq-copilot/plugin/core/analysis/featureengine/Zeit.h:96`; `FeatureEngine::trennIndex` — `eq-copilot/plugin/core/analysis/featureengine/Zeit.h:105`; `FeatureEngine::zuordnung` — `eq-copilot/plugin/core/analysis/featureengine/Zeit.h:120`; `FeatureEngine::auskunftBestimmen` — `eq-copilot/plugin/core/analysis/featureengine/Zeit.h:151`; `FeatureEngine::grenzeZwischen` — `eq-copilot/plugin/core/analysis/featureengine/Zeit.h:180`; `FeatureEngine::istLoopWrap` — `eq-copilot/plugin/core/analysis/featureengine/Zeit.h:242`; `FeatureEngine::ppqProSampleVon` — `eq-copilot/plugin/core/analysis/featureengine/Zeit.h:264`; `FeatureEngine::moeglicherStraddleIn` — `eq-copilot/plugin/core/analysis/featureengine/Zeit.h:288`; `FeatureEngine::grenzeZiehen` — `eq-copilot/plugin/core/analysis/featureengine/Zeit.h:318`; `FeatureEngine::blockProjektSpanneGueltig` — `eq-copilot/plugin/core/analysis/featureengine/Zeit.h:524`; `FeatureEngine::blockContinuousSpanneGueltig` — `eq-copilot/plugin/core/analysis/featureengine/Zeit.h:536`; `FeatureEngine::evidenzContinuousBelegen` — `eq-copilot/plugin/core/analysis/featureengine/Zeit.h:545`; `FeatureEngine::rahmenZeitBelegen` — `eq-copilot/plugin/core/analysis/featureengine/Zeit.h:568` |
| `eq-copilot/plugin/core/analysis/FeatureEngine.h` | 1516 | 67 | `schiebe` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:217`; `leeren` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:223`; `vorbereiten` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:285`; `leeren` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:295`; `passageSchiebe` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:307`; `klassenwertAmRang` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:325`; `passagePerzentil` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:341`; `vorbereiten` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:451`; `zuruecksetzen` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:526`; `nimmBlock` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:643`; `frame` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:688`; `fingerprint` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:698`; `stereoBand` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:708`; `stereoSkalare` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:711`; `stereoHatInhalt` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:717`; `setzePassagenfenster` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:749`; `loeschePassagenfenster` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:816`; `passagenfensterGesetzt` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:833`; `passagenfensterIntakt` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:836`; `passagenfensterStart` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:838`; `passagenfensterEnde` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:839`; `passagenfensterEpoche` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:841`; `nichtEndlicheSamples` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:845`; `nichtEndlicheSamplesImEvidenzfenster` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:848`; `ereignisAnzahlJetzt` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:851`; `ereignis` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:852`; `getrennteFenster` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:863`; `verworfeneFensterSamples` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:864`; `epochenwechsel` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:865`; `segmentwechsel` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:866`; `straddleVerworfen` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:867`; `ereignisseVerworfen` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:868`; `nak29Abgelehnt` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:869`; `bloeckeGesehen` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:870`; `summeFensterGesamt` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:878`; `summeFensterAktiv` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:879`; `grenzenMitGrund` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:892`; `transportEpocheJetzt` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:899`; `segmentJetzt` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:900`; `fuellstandBass` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:904`; `fuellstandHaupt` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:905`; `fuellstandLoudnessZelle` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:906`; `fuellstandKurzLoudness` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:907`; `flussHatVorgaenger` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:908`; `liveAkkuBelegteBaender` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:918`; `evidenzAkkuBelegteBaender` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:924`; `evidenzVerteilungPlaetze` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:939`; `evidenzFensterGesamtJetzt` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:948`; `evidenzFensterAktivJetzt` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:949`; `fingerprintFenster` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:966`; `stereoAkkuBelegteBaender` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:967`; `evidenzIntervallSetzen` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:984`; `evidenzIntervallJetzt` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:991`; `ereignisseEntnommen` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:999`; `liveBreiteAkkuZustand` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:1002`; `liveBreiteAkkuBelegteBaender` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:1018`; `rahmenZellenJetzt` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:1028`; `rahmenAktivZellenJetzt` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:1029`; `liveSamplesJetzt` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:1036`; `evidenzSamplesJetzt` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:1037`; `verworfeneBandfenster` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:1045`; `kFilterZustand` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:1057`; `erstesMessbaresBand` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:1068`; `erstesBandUeberKappe` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:1069`; `samplerate` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:1070`; `vorbereiten` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:1101`; `leeren` — `eq-copilot/plugin/core/analysis/FeatureEngine.h:1135` |
| `eq-copilot/plugin/core/analysis/Fft.h` | 143 | 6 | `vorbereiten` — `eq-copilot/plugin/core/analysis/Fft.h:43`; `groesse` — `eq-copilot/plugin/core/analysis/Fft.h:76`; `transformiere` — `eq-copilot/plugin/core/analysis/Fft.h:80`; `leistung` — `eq-copilot/plugin/core/analysis/Fft.h:114`; `realTeil` — `eq-copilot/plugin/core/analysis/Fft.h:133`; `imagTeil` — `eq-copilot/plugin/core/analysis/Fft.h:134` |
| `eq-copilot/plugin/core/analysis/KGewichtung.h` | 127 | 8 | `tick` — `eq-copilot/plugin/core/analysis/KGewichtung.h:44`; `zustandNullen` — `eq-copilot/plugin/core/analysis/KGewichtung.h:52`; `rbjHighShelf` — `eq-copilot/plugin/core/analysis/KGewichtung.h:63`; `rbjHighpass` — `eq-copilot/plugin/core/analysis/KGewichtung.h:80`; `shelfQ` — `eq-copilot/plugin/core/analysis/KGewichtung.h:104`; `entwerfen` — `eq-copilot/plugin/core/analysis/KGewichtung.h:112`; `zustandNullen` — `eq-copilot/plugin/core/analysis/KGewichtung.h:118`; `tick` — `eq-copilot/plugin/core/analysis/KGewichtung.h:124` |
| `eq-copilot/plugin/core/analysis/Konfidenz.h` | 194 | 3 | `klasseName` — `eq-copilot/plugin/core/analysis/Konfidenz.h:55`; `deckle` — `eq-copilot/plugin/core/analysis/Konfidenz.h:127`; `gesamtklasse` — `eq-copilot/plugin/core/analysis/Konfidenz.h:153` |
| `eq-copilot/plugin/core/analysis/LoudnessAccumulator.h` | 460 | 21 | `addiere` — `eq-copilot/plugin/core/analysis/LoudnessAccumulator.h:91`; `wert` — `eq-copilot/plugin/core/analysis/LoudnessAccumulator.h:101`; `leeren` — `eq-copilot/plugin/core/analysis/LoudnessAccumulator.h:102`; `lautheit` — `eq-copilot/plugin/core/analysis/LoudnessAccumulator.h:158`; `vorbereiten` — `eq-copilot/plugin/core/analysis/LoudnessAccumulator.h:167`; `zuruecksetzen` — `eq-copilot/plugin/core/analysis/LoudnessAccumulator.h:182`; `zelle` — `eq-copilot/plugin/core/analysis/LoudnessAccumulator.h:206`; `integriert` — `eq-copilot/plugin/core/analysis/LoudnessAccumulator.h:283`; `unsicherheitLu` — `eq-copilot/plugin/core/analysis/LoudnessAccumulator.h:321`; `kurz` — `eq-copilot/plugin/core/analysis/LoudnessAccumulator.h:354`; `zellenGesamt` — `eq-copilot/plugin/core/analysis/LoudnessAccumulator.h:372`; `bloeckeGesamt` — `eq-copilot/plugin/core/analysis/LoudnessAccumulator.h:373`; `bloeckeAbsGate` — `eq-copilot/plugin/core/analysis/LoudnessAccumulator.h:374`; `bloeckeUnterGate` — `eq-copilot/plugin/core/analysis/LoudnessAccumulator.h:375`; `bloeckeUeberGitter` — `eq-copilot/plugin/core/analysis/LoudnessAccumulator.h:379`; `bloeckeUeberOberband` — `eq-copilot/plugin/core/analysis/LoudnessAccumulator.h:382`; `bloeckeNichtEndlich` — `eq-copilot/plugin/core/analysis/LoudnessAccumulator.h:383`; `speicherBytes` — `eq-copilot/plugin/core/analysis/LoudnessAccumulator.h:387`; `binIndex` — `eq-copilot/plugin/core/analysis/LoudnessAccumulator.h:402`; `grenzBin` — `eq-copilot/plugin/core/analysis/LoudnessAccumulator.h:416`; `binMitte` — `eq-copilot/plugin/core/analysis/LoudnessAccumulator.h:424` |
| `eq-copilot/plugin/core/analysis/Statistik.h` | 187 | 6 | `zustand` — `eq-copilot/plugin/core/analysis/Statistik.h:40`; `naechste` — `eq-copilot/plugin/core/analysis/Statistik.h:42`; `blockBootstrap` — `eq-copilot/plugin/core/analysis/Statistik.h:70`; `fdrSignifikant` — `eq-copilot/plugin/core/analysis/Statistik.h:122`; `Wuerfel` — `eq-copilot/plugin/core/analysis/Statistik.h:39`; `cluster` — `eq-copilot/plugin/core/analysis/Statistik.h:163` |
| `eq-copilot/plugin/core/analysis/TruePeak.h` | 321 | 11 | `vorbereiten` — `eq-copilot/plugin/core/analysis/TruePeak.h:97`; `koeffizienten` — `eq-copilot/plugin/core/analysis/TruePeak.h:139`; `zuruecksetzen` — `eq-copilot/plugin/core/analysis/TruePeak.h:149`; `bereit` — `eq-copilot/plugin/core/analysis/TruePeak.h:159`; `tick` — `eq-copilot/plugin/core/analysis/TruePeak.h:171`; `nachlauf` — `eq-copilot/plugin/core/analysis/TruePeak.h:215`; `interpolationsfehlerDb` — `eq-copilot/plugin/core/analysis/TruePeak.h:229`; `besselI0` — `eq-copilot/plugin/core/analysis/TruePeak.h:276`; `cosKonst` — `eq-copilot/plugin/core/analysis/TruePeak.h:294`; `log10Konst` — `eq-copilot/plugin/core/analysis/TruePeak.h:304`; `berechneFir` — `eq-copilot/plugin/core/analysis/TruePeak.h:243` |
| `eq-copilot/plugin/core/analysis/Vergleichspegel.h` | 512 | 28 | `vorbereiten` — `eq-copilot/plugin/core/analysis/Vergleichspegel.h:73`; `speise` — `eq-copilot/plugin/core/analysis/Vergleichspegel.h:113`; `bereit` — `eq-copilot/plugin/core/analysis/Vergleichspegel.h:137`; `nichtEndlicheSamples` — `eq-copilot/plugin/core/analysis/Vergleichspegel.h:147`; `aufgenommeneBloecke` — `eq-copilot/plugin/core/analysis/Vergleichspegel.h:161`; `gezaehlteFrames` — `eq-copilot/plugin/core/analysis/Vergleichspegel.h:174`; `zaehlerstand` — `eq-copilot/plugin/core/analysis/Vergleichspegel.h:184`; `friereEin` — `eq-copilot/plugin/core/analysis/Vergleichspegel.h:208`; `eingefroren` — `eq-copilot/plugin/core/analysis/Vergleichspegel.h:238`; `gainGesetzt` — `eq-copilot/plugin/core/analysis/Vergleichspegel.h:239`; `gainDb` — `eq-copilot/plugin/core/analysis/Vergleichspegel.h:243`; `loeschen` — `eq-copilot/plugin/core/analysis/Vergleichspegel.h:247`; `setzeBlockmitteHakenFuerTest` — `eq-copilot/plugin/core/analysis/Vergleichspegel.h:265`; `setzeEinfrierWartetHakenFuerTest` — `eq-copilot/plugin/core/analysis/Vergleichspegel.h:270`; `audioZugNehmen` — `eq-copilot/plugin/core/analysis/Vergleichspegel.h:286`; `audioZugGeben` — `eq-copilot/plugin/core/analysis/Vergleichspegel.h:293`; `abzug` — `eq-copilot/plugin/core/analysis/Vergleichspegel.h:344`; `uebernehmenIntern` — `eq-copilot/plugin/core/analysis/Vergleichspegel.h:351`; `uebernehmen` — `eq-copilot/plugin/core/analysis/Vergleichspegel.h:367`; `nimmFrames` — `eq-copilot/plugin/core/analysis/Vergleichspegel.h:371`; `bereitIntern` — `eq-copilot/plugin/core/analysis/Vergleichspegel.h:414`; `leerenIntern` — `eq-copilot/plugin/core/analysis/Vergleichspegel.h:423`; `deltaZustand` — `eq-copilot/plugin/core/analysis/Vergleichspegel.h:482`; `sperrName` — `eq-copilot/plugin/core/analysis/Vergleichspegel.h:500`; `Vergleichspegel` — `eq-copilot/plugin/core/analysis/Vergleichspegel.h:54`; `operator=` — `eq-copilot/plugin/core/analysis/Vergleichspegel.h:59`; `Steuerzug` — `eq-copilot/plugin/core/analysis/Vergleichspegel.h:306`; `~Steuerzug` — `eq-copilot/plugin/core/analysis/Vergleichspegel.h:324` |

**52 private Deklarationen in `FeatureEngine.h` (keine zusätzlichen Funktionskörper):** `projektVorwaerts`:1166; `supportVereinen`:1169; `supportMerken`:1171; `trennIndex`:1173; `zuordnung`:1175; `auskunftBestimmen`:1177; `grenzeZwischen`:1179; `istLoopWrap`:1182; `ppqProSampleVon`:1185; `moeglicherStraddleIn`:1187; `grenzeZiehen`:1189; `blockProjektSpanneGueltig`:1191; `blockContinuousSpanneGueltig`:1193; `evidenzContinuousBelegen`:1195; `rahmenZeitBelegen`:1197; `passagenanfangNochVorn`:1199; `verarbeiteSamples`:1201; `schiebeStufe`:1203; `rechneFenster`:1206; `summeBereich`:1208; `flussSchritt`:1210; `medianDerHistorie`:1212; `ereignisAblegen`:1214; `stereoSchritt`:1216; `stereoSample`:1218; `stereoAuswerten`:1220; `stereoLeeren`:1222; `fingerprintSchritt`:1224; `fingerprintJetzt`:1226; `fingerprintLeeren`:1228; `zelleSchliessen`:1230; `lraBin`:1232; `kurzLufs`:1234; `momentanLufs`:1236; `zellenFensterSauber`:1238; `kurzfensterNeuBeginnen`:1240; `kurzTruePeak`:1242; `crestKurz`:1244; `lraLu`:1246; `lraBinMitte`:1248; `lraPerzentil`:1250; `baueFrame`:1252; `evidenzLeeren`:1254; `rahmenLeeren`:1256; `baueStempel`:1258; `fuelleLive`:1261; `fuelleEvidenz`:1263; `ringInZeitfolge`:1265; `perzentil`:1267; `fuelleVerteilung`:1269; `fuelleAbdeckungUndKonvergenz`:1271; `fuelleSkalare`:1273.

**Default/delete:** `featureengine/Vertrag.h:163` (`Transportstempel::operator==`), `:357` (`Fingerprint::operator==`), `:555` (`Bandsatz::operator==`), `:769` (`FeatureFrame::operator==`); `Vergleichspegel.h:53` (Defaultkonstruktor). `Vergleichspegel.h:325` / `:326` löschen Kopie/Zuweisung von `Steuerzug`.

### 3. Vollständige Feldpopulation der acht ausgetauschten Kernstrukturen

In `eq-copilot/plugin/core/analysis/featureengine/Vertrag.h` wurden **136 benannte Feld-/Konstantendeklarationen** der folgenden acht Strukturen gezählt: **131 Instanzfelder plus fünf statische Konstanten** (vier in `Fingerprint`, eine in `Bandsatz`). Das ist die explizit abgegrenzte Austauschfeldpopulation, keine irreführende Gesamtzahl aller privaten Ring-/Filterzustände der Kernklassen. Deren Definitionen und Resetpfade wurden mit den vollständigen 18 Kernheadern gelesen.

| Struktur | Deklarationen einschließlich genannter Konstanten | Vollständige Feldliste; Zeilen im Vertragspfad |
|---|---:|---|
| `Transportstempel` — `eq-copilot/plugin/core/analysis/featureengine/Vertrag.h:122` | 26 | `transport_epoch`:124; `continuity_segment`:125; `sequence`:126; `zeitbasis`:127; `project_sample_start_gesetzt`:129; `project_sample_start`:130; `sample_count`:132; `sample_rate`:133; `playing`:134; `recording`:135; `continuous_time_samples_gesetzt`:137; `continuous_time_samples`:138; `cycle_active`:141; `cycle_bounds_valid`:142; `cycle_start_ppq_gesetzt`:143; `cycle_start_ppq`:144; `cycle_end_ppq_gesetzt`:145; `cycle_end_ppq`:146; `cycle_derivation`:150; `input_presentation_latency_gesetzt`:152; `input_presentation_latency`:153; `output_presentation_latency_gesetzt`:154; `output_presentation_latency`:155; `gueltigkeit`:157; `process_context_present_gesetzt`:158; `process_context_present`:159 |
| `Frameversuch` — `eq-copilot/plugin/core/analysis/featureengine/Vertrag.h:249` | 2 | `sequence`:251; `verstoss`:252 |
| `Ereignis` — `eq-copilot/plugin/core/analysis/featureengine/Vertrag.h:272` | 10 | `stromSample`:275; `projektzeitGesetzt`:277; `projektSample`:278; `epoche`:281; `segment`:282; `staerke`:284; `bandZentrumHz`:285; `dauerMs`:286; `qualitaetFluss`:287; `qualitaetPeak`:288 |
| `Fingerprint` — `eq-copilot/plugin/core/analysis/featureengine/Vertrag.h:332` | 10 | `kVersion`:338; `kBaender`:340; `kChroma`:341; `kOnsets`:342; `gesetzt`:344; `version`:348; `bandEnergie`:349; `chroma`:350; `onset`:351; `fenster`:355 |
| `StereoBandwert` — `eq-copilot/plugin/core/analysis/featureengine/Vertrag.h:447` | 20 | `basisGesetzt`:458; `midDb`:459; `sideDb`:459; `seitenanteilDb`:459; `korrelationKurz`:465; `korrelationMittel`:465; `korrelationKurzGesetzt`:476; `korrelationMittelGesetzt`:477; `kohaerenzGesetzt`:480; `kohaerenz`:480; `phaseGesetzt`:482; `phaseRad`:482; `persistenz`:488; `persistenzGesetzt`:502; `perzentileGesetzt`:508; `p10Db`:509; `p50Db`:509; `p95Db`:509; `fensterDauerMs`:514; `freiheitsgrade`:515 |
| `StereoSkalare` — `eq-copilot/plugin/core/analysis/featureengine/Vertrag.h:525` | 4 | `folddownGesetzt`:527; `monoFolddownDb`:527; `balanceGesetzt`:528; `lrBalanceDb`:528 |
| `Bandsatz` — `eq-copilot/plugin/core/analysis/featureengine/Vertrag.h:537` | 6 | `gitter`:539; `encoding`:540; `werte`:541; `bitmap`:542; `saturated`:543; `baender`:545 |
| `FeatureFrame` — `eq-copilot/plugin/core/analysis/featureengine/Vertrag.h:567` | 58 | `transport`:569; `live`:570; `evidenz`:571; `evidenzFrisch`:572; `liveBreite`:583; `liveBreiteBitmap`:584; `metricsVersion`:586; `aktivitaetGesetzt`:588; `aktivitaet`:588; `lufsSGesetzt`:589; `lufsS`:589; `peakGesetzt`:590; `peakDb`:590; `crestGesetzt`:591; `crestDb`:591; `psrGesetzt`:592; `psrDb`:592; `breiteGesetzt`:593; `breite`:593; `korrelationGesetzt`:594; `korrelation`:594; `lufsIGesetzt`:600; `lufsI`:600; `lufsIUnsicherheitGesetzt`:601; `lufsIUnsicherheit`:601; `lufsIStatusGesetzt`:602; `lufsIStatus`:602; `integrationGesetzt`:618; `integrationSamples`:618; `nichtEndlichRahmen`:631; `nichtEndlichEvidenz`:632; `lufsMGesetzt`:663; `lufsM`:663; `truePeakGesetzt`:664; `truePeakDb`:664; `truePeakPassageGesetzt`:665; `truePeakPassageDb`:665; `plrGesetzt`:666; `plrDb`:666; `lraGesetzt`:667; `lraLu`:667; `crestKurzGesetzt`:668; `crestKurzDb`:668; `headroomGesetzt`:688; `headroomP10Db`:689; `headroomP50Db`:689; `headroomP95Db`:689; `headroomFenster`:690; `evidenzP10`:704; `evidenzP50`:704; `evidenzP95`:704; `evidenzFenster`:712; `abdeckungGesetzt`:727; `abdeckung`:727; `konvergenzGesetzt`:728; `konvergenz`:728; `evidenzStromStartGesetzt`:738; `evidenzStromStart`:739 |

### 4. Alle direkten Sonden-Aufrufe in den Kern

Es gibt **34 ausführbare `merkmale.*`-Aufrufstellen auf 21 unterschiedliche FeatureEngine-Methoden**, darunter die Testzugriffe im Header. Ein verschachtelter Aufruf zählt zweimal; kommentierte Aufrufe zählen nicht. Hinzu kommen **zwei freie Konfidenz-Aufrufe** an derselben Quellzeile: insgesamt **36 explizite Funktionsaufrufe** aus `sonde/**` in die hier geprüften Kernheader. Aggregate-/Defaultinitialisierung ist keine zusätzliche explizite Fachfunktionsauswertung.

| Datei:Zeile | Aufruf |
|---|---|
| `eq-copilot/plugin/sonde/SondeProcessor.cpp:554` | `merkmale.vorbereiten` |
| `eq-copilot/plugin/sonde/SondeProcessor.cpp:563` | `merkmale.zuruecksetzen` |
| `eq-copilot/plugin/sonde/SondeProcessor.cpp:614` | `merkmale.getrennteFenster` |
| `eq-copilot/plugin/sonde/SondeProcessor.cpp:615` | `merkmale.straddleVerworfen` |
| `eq-copilot/plugin/sonde/SondeProcessor.cpp:616` | `merkmale.nimmBlock` |
| `eq-copilot/plugin/sonde/SondeProcessor.cpp:617` | `merkmale.getrennteFenster` |
| `eq-copilot/plugin/sonde/SondeProcessor.cpp:618` | `merkmale.straddleVerworfen` |
| `eq-copilot/plugin/sonde/SondeProcessor.cpp:638` | `merkmale.frame` |
| `eq-copilot/plugin/sonde/SondeProcessor.cpp:760` | `merkmale.evidenzIntervallJetzt` |
| `eq-copilot/plugin/sonde/SondeProcessor.cpp:761` | `merkmale.evidenzIntervallSetzen` |
| `eq-copilot/plugin/sonde/SondeProcessor.cpp:762` | `merkmale.evidenzIntervallJetzt` |
| `eq-copilot/plugin/sonde/SondeProcessor.cpp:779` | `merkmale.ereignisseVerworfen` |
| `eq-copilot/plugin/sonde/SondeProcessor.cpp:785` | `merkmale.ereignisAnzahlJetzt` |
| `eq-copilot/plugin/sonde/SondeProcessor.cpp:789` | `merkmale.ereignis` |
| `eq-copilot/plugin/sonde/SondeProcessor.cpp:834` | `merkmale.stereoHatInhalt` |
| `eq-copilot/plugin/sonde/SondeProcessor.cpp:836` | `merkmale.stereoBand` |
| `eq-copilot/plugin/sonde/SondeProcessor.cpp:837` | `merkmale.stereoSkalare` |
| `eq-copilot/plugin/sonde/SondeProcessor.cpp:859` | `merkmale.evidenzIntervallJetzt` |
| `eq-copilot/plugin/sonde/SondeProcessor.cpp:860` | `merkmale.evidenzIntervallSetzen` |
| `eq-copilot/plugin/sonde/SondeProcessor.cpp:861` | `merkmale.evidenzIntervallJetzt` |
| `eq-copilot/plugin/sonde/SondeProcessor.cpp:870` | `merkmale.evidenzIntervallSetzen` |
| `eq-copilot/plugin/sonde/SondeProcessor.cpp:870` | `merkmale.evidenzIntervallJetzt` |
| `eq-copilot/plugin/sonde/SondeProcessor.cpp:893` | `merkmale.ereignisseEntnommen` |
| `eq-copilot/plugin/sonde/SondeProcessor.cpp:941` | `merkmale.frame` |
| `eq-copilot/plugin/sonde/SondeProcessor.cpp:943` | `merkmale.summeFensterGesamt` |
| `eq-copilot/plugin/sonde/SondeProcessor.cpp:944` | `merkmale.summeFensterAktiv` |
| `eq-copilot/plugin/sonde/SondeProcessor.cpp:945` | `merkmale.evidenzFensterGesamtJetzt` |
| `eq-copilot/plugin/sonde/SondeProcessor.cpp:946` | `merkmale.evidenzFensterAktivJetzt` |
| `eq-copilot/plugin/sonde/SondeProcessor.h:296` | `merkmale.evidenzIntervallJetzt` |
| `eq-copilot/plugin/sonde/SondeProcessor.h:342` | `merkmale.getrennteFenster` |
| `eq-copilot/plugin/sonde/SondeProcessor.h:347` | `merkmale.grenzenMitGrund` |
| `eq-copilot/plugin/sonde/SondeProcessor.h:373` | `merkmale.frame` |
| `eq-copilot/plugin/sonde/SondeProcessor.h:378` | `merkmale.nak29Abgelehnt` |
| `eq-copilot/plugin/sonde/SondeProcessor.h:384` | `merkmale.frame` |
| `eq-copilot/plugin/sonde/SondeProcessor.cpp:826` | `nakama::analyse::gesamtklasse` |
| `eq-copilot/plugin/sonde/SondeProcessor.cpp:826` | `nakama::analyse::klasseName` |

### 5. Weitere Pluginquellen — 20 Dateien

| Pfad | Dateizeilen | Tatsächlich gelesener Bereich |
|---|---:|---|
| `eq-copilot/plugin/core/ipc/InterventionsRing.h` | 232 | 43–98 |
| `eq-copilot/plugin/core/ipc/TelemetryClient.cpp` | 812 | 1–195 |
| `eq-copilot/plugin/dsp/DspBankPool.cpp` | 285 | 1–80 |
| `eq-copilot/plugin/dsp/DspBankPool.h` | 309 | 1–200 |
| `eq-copilot/plugin/dsp/DspFilter.h` | 369 | 30–115; 224–368 |
| `eq-copilot/plugin/dsp/DspKern.cpp` | 1104 | 1–196; 322–335; 427–1104 |
| `eq-copilot/plugin/dsp/DspKern.h` | 451 | vollständig, 1–451 |
| `eq-copilot/plugin/dsp/DspProgramm.cpp` | 272 | vollständig, 1–272 |
| `eq-copilot/plugin/dsp/DspProgramm.h` | 222 | vollständig, 1–222 |
| `eq-copilot/plugin/sonde/SondeFactory.cpp` | 11 | vollständig, 1–11 |
| `eq-copilot/plugin/sonde/SondeProcessor.cpp` | 1446 | vollständig, 1–1446 |
| `eq-copilot/plugin/sonde/SondeProcessor.h` | 562 | vollständig, 1–562 |
| `eq-copilot/plugin/src/AnalyseEngine.cpp` | 1132 | vollständig, 1–1132 |
| `eq-copilot/plugin/src/AnalyseEngine.h` | 374 | vollständig, 1–374 |
| `eq-copilot/plugin/src/HoerMarkierung.h` | 816 | vollständig, 1–816 |
| `eq-copilot/plugin/src/PluginEditor.cpp` | 1665 | 1289–1314 |
| `eq-copilot/plugin/src/SourcesModel.cpp` | 1947 | 1554–1605 |
| `eq-copilot/plugin/src/prozessor/Analyse.cpp` | 1438 | vollständig, 1–1438 |
| `eq-copilot/plugin/vertrag/NakamaEvidenz.cpp` | 606 | vollständig, 1–606 |
| `eq-copilot/plugin/vertrag/NakamaTelemetrie.cpp` | 761 | 433–556 |

Bei `DspKern.cpp` wurden 197–321 und 336–426 nicht als Messkern geprüft; bei `DspFilter.h` bleibt der übrige Filterentwurf außerhalb der Messkante. `DspBankPool` wurde nur hinsichtlich des für die Messung genutzten Zustands gelesen. `SourcesModel` und `PluginEditor` sind ausschließlich Zahlenverbraucher in diesem Audit. `HoerMarkierung.*` löst im aktuellen Baum auf den gelesenen Header auf; eine zusätzliche `.cpp` ist dafür nicht erforderlich.

### 6. Tests — neun Golden-Dateien und drei ergänzende C++-Tests

| Pfad | Dateizeilen | Tatsächlich gelesener Bereich |
|---|---:|---|
| `eq-copilot/plugin/tests/AnalysisGoldenTestMain.cpp` | 2961 | vollständig, 1–2961 |
| `eq-copilot/plugin/tests/DspGoldenTestMain.cpp` | 4586 | 184–383; 425–538; 1556–2137; 2900–3406; 3890–4062 |
| `eq-copilot/plugin/tests/GoldenTestMain.cpp` | 883 | vollständig, 1–883 |
| `eq-copilot/plugin/tests/LoudnessGoldenTestMain.cpp` | 670 | vollständig, 1–670 |
| `eq-copilot/plugin/tests/Sonde012LoudnessSourceTest.cpp` | 334 | vollständig, 1–334 |
| `eq-copilot/plugin/tests/Sonde013DynamicsTest.cpp` | 1326 | vollständig, 1–1326 |
| `eq-copilot/plugin/tests/Sonde013EventWireTest.cpp` | 2460 | 275–427; 2058–2459 |
| `eq-copilot/plugin/tests/Sonde013ExperimentGoldenTest.cpp` | 285 | vollständig, 1–285 |
| `eq-copilot/plugin/tests/Sonde013FingerprintGoldenTest.cpp` | 356 | vollständig, 1–356 |
| `eq-copilot/plugin/tests/Sonde013PrePostGoldenTest.cpp` | 575 | vollständig, 1–575 |
| `eq-copilot/plugin/tests/Sonde013StereoGoldenTest.cpp` | 1329 | vollständig, 1–1329 |
| `eq-copilot/plugin/tests/Sonde013TruePeakGoldenTest.cpp` | 634 | vollständig, 1–634 |

Der Broker-Gegenbeleg `broker/tests/sonde013_experiment.rs:481–527` steht im folgenden Brokerbereich. Testzahlen wurden nicht aus der Zahl von `pruefe`-Texttreffern abgeleitet; Schleifen und unterschiedliche Testhelfer machen solche Zahlen unzuverlässig.

### 7. Numerische Broker-Verbraucher und Hypothesenpfade — 18 Dateien

| Pfad | Dateizeilen | Tatsächlich gelesener Bereich |
|---|---:|---|
| `broker/src/coordinator/evidenz.rs` | 528 | 253–385 |
| `broker/src/coordinator/experiment.rs` | 1727 | 310–610; 1480–1727 |
| `broker/src/coordinator/experiment_verdrahtung.rs` | 1892 | 839–1235 |
| `broker/src/coordinator/hypothese/befund.rs` | 484 | 1–382 |
| `broker/src/coordinator/hypothese/determinismus.rs` | 120 | vollständig, 1–120 |
| `broker/src/coordinator/hypothese/eingang.rs` | 179 | 1–167 |
| `broker/src/coordinator/hypothese/enthaltung.rs` | 203 | vollständig, 1–203 |
| `broker/src/coordinator/hypothese/klassenwahl.rs` | 240 | 1–172 |
| `broker/src/coordinator/hypothese/messung.rs` | 556 | 1–330 |
| `broker/src/coordinator/hypothese/rechnung.rs` | 523 | 1–422 |
| `broker/src/coordinator/hypothese/screening.rs` | 813 | 1–345 |
| `broker/src/coordinator/hypothese/vertrag.rs` | 354 | 1–319 |
| `broker/src/coordinator/hypothese/zusammenhang.rs` | 630 | vollständig, 1–630 |
| `broker/src/coordinator/hypothese_verdrahtung.rs` | 1330 | 484–634; 754–825; 1183–1273; 1312–1327 |
| `broker/src/coordinator/invalidierung.rs` | 285 | 185–222 |
| `broker/src/coordinator/vergleichbarkeit.rs` | 403 | 281–355 |
| `broker/src/telemetrie.rs` | 1096 | 60–114; 572–761; 928–1096 |
| `broker/tests/sonde013_experiment.rs` | 1074 | 481–527 |

Diese Erweiterung der Phase folgt den Messwerten bis zur Interpretation. Außerhalb der genannten Bereiche wurden die Brokerdateien nicht vollständig auditiert. `hypothese/mod.rs` und `hypothese/testhilfe.rs` wurden lediglich inventarisiert; die Test-/Lebenslaufteile nach den ausgewiesenen Produktionsrechnungen sind kein stillschweigend abgedeckter Scope.

### 8. Python-Werkzeuge — ganze Dateiinventur, abgegrenzte Messrechnungen

Die gesamte Population unter `tools/eq-copilot/*.py` umfasst **27 Dateien und 447 AST-Funktionsdefinitionen** einschließlich verschachtelter Definitionen. **14 Dateien** wurden vollständig im ausführbaren Code gelesen (**108 Definitionen**); **vier weitere** in ihren numerischen Bereichen. **Neun** nicht zur Messrechnung gehörende Werkzeuge wurden nur inventarisiert, nicht im Funktionskörper geprüft. Die AST-Gesamtzählung ist deshalb ausdrücklich keine Behauptung, alle 447 Funktionen fachlich auditiert zu haben.

| Python-Datei | Definitionen der ganzen Datei | Gelesener Messscope | Funktionsanker im gelesenen Scope |
|---|---:|---|---|
| `tools/eq-copilot/erzeuge_aux_spike_fixtures.py` | 2 | vollständiger ausführbarer Code | `erzeuge`:40; `main`:62 |
| `tools/eq-copilot/erzeuge_bandgitter.py` | 13 | vollständiger ausführbarer Code | `hex64`:86; `aus_hex64`:91; `dezimal`:95; `zahlenblock`:100; `x_bereich`:110; `feines_gitter`:117; `grobes_gitter`:154; `pruefe`:214; `schreibe`:287; `main`:295; `__init__`:197; `gleich`:201; `wahr`:207 |
| `tools/eq-copilot/erzeuge_bandgitter_header.py` | 6 | vollständiger ausführbarer Code | `lade`:41; `hexblock`:46; `zahlblock`:55; `baue`:62; `pruefe_selbst`:139; `main`:180 |
| `tools/eq-copilot/erzeuge_envelope_fixtures.py` | 8 | nur Inventur; außerhalb der Messrechnung | keine fachlich gelesenen Definitionskörper |
| `tools/eq-copilot/erzeuge_fb_fixtures.py` | 31 | 54–128 | `adresse`:54; `bitmap`:67; `baender`:80; `band_stereo`:90; `frame`:115 |
| `tools/eq-copilot/erzeuge_fixtures.py` | 7 | vollständiger ausführbarer Code | `_schreibe_wav`:38; `_pegel`:57; `baue_signale`:61; `lade_analyze`:146; `main`:167; `forme`:101; `strikt`:222 |
| `tools/eq-copilot/erzeuge_p4_korpus.py` | 4 | vollständiger ausführbarer Code | `q`:85; `_kanonisch`:280; `baue`:288; `main`:363 |
| `tools/eq-copilot/erzeuge_p5_korpus.py` | 5 | vollständiger ausführbarer Code | `sitzung`:147; `quelle`:218; `_kanonisch`:759; `baue`:765; `main`:818 |
| `tools/eq-copilot/erzeuge_quantisierung.py` | 11 | vollständiger ausführbarer Code | `hex64`:63; `hex32`:67; `rund_weg_von_null`:74; `quantisiere_i16`:96; `quantisiere_f32`:108; `i16_vektoren`:134; `f32_vektoren`:177; `inhalt`:213; `pruefe`:264; `main`:308; `wahr`:267 |
| `tools/eq-copilot/erzeuge_state_fixtures.py` | 63 | nur Inventur; außerhalb der Messrechnung | keine fachlich gelesenen Definitionskörper |
| `tools/eq-copilot/erzeuge_testsong.py` | 5 | vollständiger ausführbarer Code | `idx`:70; `vel`:74; `baue`:79; `audit`:194; `main`:264 |
| `tools/eq-copilot/erzeuge_v3_fixtures.py` | 30 | 3591–3865 | `wire_zahl`:3591; `zahlklassen_wire`:3639; `evidenz_0p01_paar_wire`:3723; `bits`:3643; `eintrag`:3646; `quant`:3777; `schnappschuss`:3784 |
| `tools/eq-copilot/pruefe_fbs_feldids.py` | 9 | nur Inventur; außerhalb der Messrechnung | keine fachlich gelesenen Definitionskörper |
| `tools/eq-copilot/pruefe_flatc_drift.py` | 9 | nur Inventur; außerhalb der Messrechnung | keine fachlich gelesenen Definitionskörper |
| `tools/eq-copilot/pruefe_host_capabilities.py` | 4 | nur Inventur; außerhalb der Messrechnung | keine fachlich gelesenen Definitionskörper |
| `tools/eq-copilot/pruefe_installer_gegenpfad.py` | 8 | nur Inventur; außerhalb der Messrechnung | keine fachlich gelesenen Definitionskörper |
| `tools/eq-copilot/pruefe_installer_manifest.py` | 53 | nur Inventur; außerhalb der Messrechnung | keine fachlich gelesenen Definitionskörper |
| `tools/eq-copilot/pruefe_ipc_last.py` | 3 | vollständiger ausführbarer Code | `probe_pipename`:55; `main`:59; `pruefe`:148 |
| `tools/eq-copilot/pruefe_kern_identitaetsfrei.py` | 23 | nur Inventur; außerhalb der Messrechnung | keine fachlich gelesenen Definitionskörper |
| `tools/eq-copilot/pruefe_p4_korpus.py` | 15 | vollständiger ausführbarer Code | `_lade`:118; `_mengen`:129; `_bein_bekannt`:146; `_quelle_pruefen`:160; `_pruefe_wahrheiten`:179; `_passt`:200; `_kennzahlen`:219; `_riegel`:271; `_enum_lesen`:314; `_vertragsmengen_pruefen`:326; `main`:353; `_selbsttest`:471; `pruefe`:475; `fall`:483; `kennzahl`:553 |
| `tools/eq-copilot/pruefe_p5_korpus.py` | 19 | vollständiger ausführbarer Code | `kennzahlen`:100; `_passt`:203; `riegel`:212; `gegenbeispiele`:240; `schwelle_suchen`:274; `produktschwelle`:314; `offene_luecken`:348; `p4_luecke`:399; `_voraussetzung`:433; `_faelle_bilden`:480; `main`:618; `selbsttest`:781; `pruefe`:790; `fall`:796; `lauf`:990; `befund`:992; `lauf2`:1058; `gegenfall`:1108; `luecken_korpus`:1129 |
| `tools/eq-copilot/pruefe_session_soak.py` | 50 | 300–405; 814–1277 | `_windows_speicher_api`:351; `_fehlmessung`:358; `rss_messung`:363; `speicherpunkt`:390; `urteile`:814; `__init__`:321; `oeffne`:330; `working_set`:334; `schliesse`:341; `letzter_fehler`:344; `pruefe`:838; `messung_fehlt`:854; `feld`:869; `pflicht`:878; `wachstum`:1126; `merkmal`:1142; `merkmal_vollstaendig`:1146; `gueltig_gemessen`:1177 |
| `tools/eq-copilot/pruefe_sonde012_sources_latency.py` | 1 | vollständiger ausführbarer Code | `main`:23 |
| `tools/eq-copilot/pruefe_v2_schemas.py` | 1 | nur Inventur; außerhalb der Messrechnung | keine fachlich gelesenen Definitionskörper |
| `tools/eq-copilot/pruefe_v3_vertrag.py` | 50 | 2001–2211 | `_konstanten_aus_kern`:2001; `pruefe_metrikregister`:2028; `pruefe_comparability_schwellen`:2110; `pruefe_experiment_belegung`:2186 |
| `tools/eq-copilot/smf.py` | 14 | vollständiger ausführbarer Code | `ascii_text`:20; `vlq`:26; `tempo_meta`:121; `timesig_meta`:126; `keysig_meta`:131; `write`:136; `__init__`:41; `cc`:49; `note`:65; `marker`:75; `at`:80; `chord`:83; `_events`:90; `chunk`:111 |
| `tools/eq-copilot/verify_testsong.py` | 3 | vollständiger ausführbarer Code | `melde`:42; `datei_events`:50; `main`:69 |

Bei einer Bereichsgrenze können vorbereitende Aufrufer außerhalb des Ausschnitts liegen; daraus wird kein Runner-Audit abgeleitet. Nicht inventierte oder absichtlich sabotierte Repoquellen wurden als Bedrohungsmodell nicht benutzt.

### 9. Fixtures — 18 Dateien, 26 P4-Fälle, 27 P5-Sitzungen, neun WAV-Header

| Pfad | Lesetiefe |
|---|---|
| `eq-copilot/fixtures/p4-korpus/MANIFEST.json` | Kontext/Struktur; kein Prüfgegenstand |
| `eq-copilot/fixtures/p4-korpus/referenz.json` | alle technischen Fallobjekte; zusammen 26 Fälle (6 Referenz, 6 Stereo, 8 Vergleich, 6 Zeitachse) |
| `eq-copilot/fixtures/p4-korpus/stereo.json` | alle technischen Fallobjekte; zusammen 26 Fälle (6 Referenz, 6 Stereo, 8 Vergleich, 6 Zeitachse) |
| `eq-copilot/fixtures/p4-korpus/vergleich.json` | alle technischen Fallobjekte; zusammen 26 Fälle (6 Referenz, 6 Stereo, 8 Vergleich, 6 Zeitachse) |
| `eq-copilot/fixtures/p4-korpus/zeitachse.json` | alle technischen Fallobjekte; zusammen 26 Fälle (6 Referenz, 6 Stereo, 8 Vergleich, 6 Zeitachse) |
| `eq-copilot/fixtures/p5-korpus/MANIFEST.json` | Kontext/Struktur; kein Prüfgegenstand |
| `eq-copilot/fixtures/p5-korpus/sitzungen.json` | alle 27 technischen Sitzungsobjekte; Erzeuger und Prüfer gelesen |
| `eq-copilot/fixtures/diag-haerte.wav` | RIFF/PCM-Header und Datenlänge; 48 kHz, stereo, 24 Bit, 960000 Frames / 20 s; keine Sampleanalyse |
| `eq-copilot/fixtures/diag-mulm-halb.wav` | RIFF/PCM-Header und Datenlänge; 48 kHz, stereo, 24 Bit, 960000 Frames / 20 s; keine Sampleanalyse |
| `eq-copilot/fixtures/diag-mulm.wav` | RIFF/PCM-Header und Datenlänge; 48 kHz, stereo, 24 Bit, 960000 Frames / 20 s; keine Sampleanalyse |
| `eq-copilot/fixtures/diag-scoop-mitte.wav` | RIFF/PCM-Header und Datenlänge; 48 kHz, stereo, 24 Bit, 960000 Frames / 20 s; keine Sampleanalyse |
| `eq-copilot/fixtures/diag-wander-ton.wav` | RIFF/PCM-Header und Datenlänge; 48 kHz, stereo, 24 Bit, 960000 Frames / 20 s; keine Sampleanalyse |
| `eq-copilot/fixtures/pink-minus20.wav` | RIFF/PCM-Header und Datenlänge; 48 kHz, stereo, 24 Bit, 960000 Frames / 20 s; keine Sampleanalyse |
| `eq-copilot/fixtures/resonanz-116hz.wav` | RIFF/PCM-Header und Datenlänge; 48 kHz, stereo, 24 Bit, 960000 Frames / 20 s; keine Sampleanalyse |
| `eq-copilot/fixtures/sinus-1k-minus12.wav` | RIFF/PCM-Header und Datenlänge; 48 kHz, stereo, 24 Bit, 960000 Frames / 20 s; keine Sampleanalyse |
| `eq-copilot/fixtures/stille-mit-tick.wav` | RIFF/PCM-Header und Datenlänge; 48 kHz, stereo, 24 Bit, 960000 Frames / 20 s; keine Sampleanalyse |
| `eq-copilot/fixtures/golden-referenz.json` | vier Signale: Skalare, acht Bandobjekte je Signal und Resonanzlisten gelesen; LTAS-Arrays mit je 221 Werten strukturell samt Länge/Rändern, nicht jede der 1768 einzelnen Zahlen nachgerechnet |
| `eq-copilot/fixtures/v3/evidenz-0p01-paar-wire-v1.json` | vollständige technische Feldstruktur des 0,01-dB-Paars; sechs 221er-Arrays und ihre Werte-/Encodingbeziehung gelesen |

Die gespeicherten Referenzen wurden nicht neu erzeugt. Unter den neun WAVs wurden keine anderen als die vom gelesenen Fixture-Erzeuger benannten Eingangssignale hinzugezogen.

### 10. Kontext und nicht gelesene beziehungsweise fehlende Pfade

Die **11 Kontext-/Skillpfade** des Bereichsprotokolls sind: `.agents/skills/nakama-plugin-quality/SKILL.md`; `CLAUDE.md`; `docs/FL-Nakama-Sonden-Design-Entwurf.md`; `docs/audits/2026-09-12-code-review-2/UEBERGABE-AN-CLAUDE.md`; `docs/beweise/NAK-283.md`; `docs/beweise/SONDE-014.md`; `docs/gesundheit/KONZEPT.md`; `docs/gesundheit/abdeckungskarte.md`; `docs/gesundheit/lebenslauf-audit.md`; `docs/offene-punkte.md`; `docs/plugin-wissen.md`. `CLAUDE.md` wurde zum Einstieg vollständig gelesen. KONZEPT §3/§4.3, Abdeckungskarte, Lebenslaufaudit und Vorgängerübergabe wurden als Klassifikations-/Gegenbelegquellen gelesen. Für den Entwurf wurden die technischen Abschnitte zu Messung, Stereo, Evidenz, Vergleich und Phasen 2–5 herangezogen; diese Dateien werden nicht als Defektfläche behandelt.

**Zusätzliche gezielte Kontextabfragen:** `docs/beweise/SONDE-013.md` (Messmatrizen, insbesondere M-07:476), `docs/beweise/NAK-283.md` (M-44…M-60, M-73/M-74/M-76 und Etappe-5-Bezug), `docs/beweise/SONDE-014.md:729–765;807–815`, `docs/plan/plan.json` (zugehörige SONDE-013/014-Einträge), Registersuche in `docs/offene-punkte.md` sowie die vorhandenen Phase-01…05-Abschnitte von `BEFUNDE.md`. Das sind selektive Anforderungs-/Dublettenabfragen; keine vollständige Lektüre aller Pläne oder historischen Rohberichte.

**Nicht gelesene Bereiche:** alle Produkt- und Testdateien außerhalb der obigen Pfad-/Bereichsliste; insbesondere `dsp/DspRtWache.h`, `dsp/DspSvf.h` sowie DSP-Programm-/Filterteile außerhalb der Messkante, die neun als „nur Inventur“ ausgewiesenen Python-Werkzeuge, weitere Broker-Lebenslaufteile, andere Fixturefamilien und historische Testrohlogs. Bei `golden-referenz.json` wurden die vollständigen numerischen LTAS-Arrays nicht manuell einzeln verifiziert. Bei allen WAVs bleibt der gesamte PCM-Inhalt ohne Neuberechnung.

**Fehlende beziehungsweise externe Quellen:** Der im Referenzwerkzeug erwartete Repo-Pfad `tools/analyze-track.py` fehlt. Der dokumentierte alternative Pfad `C:/Users/phili/FL-Studio/tools/analyze-track.py` existiert und wurde in Zeilen 1–400 von 492 gelesen; 401–492 nicht. Er benutzt unabhängige Bibliotheksverfahren für Loudness/Resampling, ist jedoch nicht durch den gebundenen Nakama-HEAD fixiert. Seine Nutzung erklärt die Referenzherkunft und ist kein zusätzlicher Registerbefund. Die offiziellen EBU-Transienten-Testdateien wurden im vorgegebenen Fixturebestand nicht gefunden und nicht beschafft (bekannt NAK-293). **Keiner der für die Messkern-Quellenprüfung benötigten Produktpfade war unlesbar.**

### 11. Abschlussbindung

- Quellenprüfung und Berichtsfassung bis **16.09.2026, 00:22:46 MESZ**; Abschlusskontrolle **00:24:21 MESZ / 15.09.2026, 22:24:21 UTC**. Gesamtdauer bis zur belegten Abschlusskontrolle: **55 Minuten 53 Sekunden**.
- HEAD vor und nach der Quellenprüfung: **`397067060d47074e67d1417ced8063fce8d5025d`** auf `master`. Abschlusskontrolle nach dem Schreiben: gleicher HEAD; kein Staging, Commit oder Push.
- Eigene Schreibmenge: ausschließlich `docs/audits/2026-09-15-tiefenaudit/roh/phase-06-messkern.md` und der neue Phase-06-Anhang in `docs/audits/2026-09-15-tiefenaudit/BEFUNDE.md`.
- Geschützter Ausgangspräfix von `BEFUNDE.md`: **46622 Bytes**, SHA-256 **`e8cf74c5fc94112214be654247dd282021799563c8379b4ee7a17d8a4603318f`**. Abschlusskontrolle: Präfix bytegleich, Tabellen wortgleich, genau ein Phase-06-Abschnitt mit drei Befundzeilen; Python-Leseprüfung Exit 0. `git diff --check` Exit 0; Sammelbericht-Diff **12 hinzugefügte, 0 entfernte Zeilen**. Die fremden untracked Pfade sind unverändert vorhanden.
- Kein aktuelles Test-PASS wird behauptet. Der Lauf ist eine abgeschlossene lesende Prüfung der ausgewiesenen Population mit den ausdrücklich ausgewiesenen Grenzen.
