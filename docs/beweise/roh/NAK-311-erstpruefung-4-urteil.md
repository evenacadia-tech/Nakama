URTEIL: PASS — kein DEFEKT

# NAK-311 Erstprüfung 4 — Etappe 4 (Auto-Gain im Monobus, Berichtsgrenze, Steuerraten-Begründung, Recall)

Prüfer: frischer, lesender Prüfer, 20.09.2026. Prüfstufe T2.
HEAD zu Beginn: `c5c00591b2e67a843f3b0cf891290b0a505782f3`.

Prüfbereich: `git diff df14a648...b0e4d452` über die 16 Auftragsdateien,
16 Dateien, 1669 Einfügungen, 32 Löschungen. Der Arbeitsbaum ist gleich ZIEL:
`git diff --stat b0e4d452 HEAD -- eq-copilot/ tools/` ist leer, über den ganzen
Baum nennt der Diff nur `docs/PLAN-STAND.md`, `docs/beweise/NAK-311.md` und den
Prüfauftrag. `git status --short` zeigt außerhalb von `docs/` nichts Verändertes,
nur zwei ticketfremde unversionierte Ordner (`briefing-hub/`, `nimbalyst-local/`).
Die Etappe besteht aus zwei Produktcommits: `48f50eca` (Teil a) und `34953955`
(Teil b); zwischen Teil a und ZIEL ändern sich nur `DspProgramm.h`,
`NakamaTransaktion.cpp` und `NakamaTransaktion.h`.

Eigene Läufe am Zielstand. Alle Prüfbinaries tragen 20.09.2026 03:54, die
jüngste Quelle 03:08:54 — Binary jünger als Quelle (NAK-230 geprüft):

| Bein | Binary | Ergebnis |
|---|---|---|
| B6 `EqCopDspGoldenTest` | 03:54:30 | Exit 0, 355 geprüft, 0 Fehler, DSP-GOLDEN OK |
| A16 `EqCopProbeeqNullTest` | 03:54:22 | Exit 0, 114 Prüfungen ok, 0 Fehler |
| B3c `EqCopSchemaTest` | 03:54:26 | Exit 0, 157 bestanden, 0 gescheitert |
| B7 `EqCopTransactionTest` | 03:54:31 | Exit 0, 187 geprüft, 0 Fehler, TRANSAKTION OK |
| B1 `EqCopIdentityTest` | 03:54:23 | Exit 0, 120 Prüfungen ok, 0 Fehler |
| A14 `EqCopStateMigrationTest` | 03:54:24 | Exit 0, 291 Prüfungen ok, 0 Fehler |
| A8 `erzeuge_v3_fixtures.py --pruefen` | — | Exit 0, 406 Dateien bytegleich zur Neuerzeugung |
| A5 `pruefe_v3_vertrag.py --abdeckung` | — | Exit 0, kein `$defs` ohne Negativfixture |

Keine dieser Ausgaben enthält eine harte Fehlerzeile.

## (a) R-311-3 — Auto-Gain im Monobus (T3-16-04, M-57 bis M-66)

**Die Monokaskade ist gebaut, wie §7.4 sie beschreibt, und sie stimmt mit dem
Kern überein.** `leiteAutoGainAb` (`eq-copilot/plugin/dsp/DspProgramm.cpp:115-164`)
zweigt bei `p.kanaele == 1` ab, startet je Gitterstelle mit dem Paar
`(a_L, a_R) = (1, 1)`, läuft über `p.baender` in Slotreihenfolge, nimmt für
jedes aktive Band dieselbe `bandRuheAntwort` wie der Zweikanalzweig
(`:128` gegen `:178`) und führt `mid` und `side` über dieselbe M/S-Rückführung
wie `DspKern::verarbeiteBand` (`DspKern.cpp:644`, `:732-733`). Ausgewertet wird
`std::norm (aLinks)` (`:153`). Die beiden Kurzschlüsse stehen davor und
dahinter unverändert: `! p.irgendeinBandAktiv() || p.samplerate <= 0.0`
(`:96`) und die zwei Wachen `! (monoMittel > 0.0) || ! std::isfinite (monoMittel)`
sowie `monoMittel == 1.0` (`:161-162`), wortgleich zu `:201-202`.

**Die Annahme der Formel trifft den Kern auf jedem Weg.** Der Monoeingang landet
in beiden Komponenten an genau einer Stelle — `verarbeiteStueck`,
`dryR[i] = (numKanaele > 1) ? … : dryL[i]` (`DspKern.cpp:1034`) —, und
geschrieben wird der zweite Kanal an genau einer Stelle,
`if (numKanaele > 1) { … kanaele[1][i] = fr; }` (`:1278-1283`). Beide liegen
außerhalb jeder Pfadverzweigung, also gelten sie für Rampe, Crossfade, Hörhalt,
Candidate, Mono-Bass, Width und Hörmatrix gleichermaßen; `verarbeiteBand`,
`verarbeiteBank` und `verarbeitePfad` lesen `numKanaele` überhaupt nicht. Die
M/S-Stufe mit Width und Mono-Bass läuft VOR den acht Bändern
(`:797-824` gegen `:836-837`), bei L = R ist dort `s` exakt 0 und beide
Komponenten bleiben gleich; hinter den Bändern mischen Auto-Gain, Mix und
Output-Trim nur je Kanal (`:846-865`). Ein `right`-Band kann deshalb im Monobus
nur über ein SPÄTERES `mid`- oder `side`-Band in den geschriebenen Kanal
zurückwirken — genau das, was die Kaskade abbildet und die Faltung nicht
abbilden könnte.

**Kanalzahl an Kern und Transaktionskern, vor der Neupublikation.**
`SondeProcessor::prepareToPlay` misst `getTotalNumInputChannels()` EINMAL
(`SondeProcessor.cpp:244`) und gibt denselben Wert an `dspKern->bereiteVor`
(`:267`) und `transaktion->setzeSamplerate` (`:270`), beide unter
`getCallbackLock()` und `zustandSchloss` (`:260-261`) und beide VOR
`publiziereWirksam` (`:272`) und `publizierePreview` (`:273`). `dspBericht`
nimmt dasselbe `zustandSchloss` (`:1300`), ein zerrissenes Lesen ist damit
ausgeschlossen. Der Prozessor deklariert genau EINEN Eingangsbus
(`:133-135`), `getTotalNumInputChannels()` ist also die Kanalzahl des
Hauptbusses; `processBlock` fährt mit `std::min (puffer.getNumChannels(), 2)`
(`:355`), und das Layout lässt nur gleiche Ein- und Ausgangsbelegung zu, also
ist die gefahrene Zahl dieselbe wie die gemessene. Die beiden einzigen
Produktaufrufer von `baueProgramm` sind `DspKern::baueVor` (`DspKern.cpp:309`,
liest `kanalzahl`) und `baueBericht` (`NakamaTransaktion.cpp:817`, liest
`tk.kanaele()`). `baueVor` und `publiziereVorbau` liegen in derselben
Transaktionsstufe unter demselben Schloss, ein Vorbau mit veralteter Kanalzahl
kann nicht publiziert werden. Ein Ladestart erzeugt keinen neuen
Transaktionskern (`SondeProcessor.cpp:149` ist der einzige Konstruktionsort).
Zwei Wege lassen eine VERALTETE Zahl stehen — `releaseResources` und ein
`prepareToPlay` mit `sichereRate == 0` oder `maxBlock <= 0`; sie lassen sie bei
BEIDEN Lesestellen gleichzeitig stehen und bilden keine Divergenz, sind aber von
keiner Zeile abgedeckt (LÜCKE L-1 und L-2 unten).

**Zweikanalzweig textgleich, Hexgolden vom unveränderten Kern.** Selbst gemessen:
`leiteAutoGainAb` am ZIEL enthält jede Zeile der Basisfassung unverändert, der
Diff der Funktion Basis gegen Ziel trägt ausschließlich Einfügungen (52 gegen
120 Zeilen, keine gelöschte oder geänderte Zeile). Die Herkunft der Hexwerte ist
in `docs/beweise/roh/NAK-311-etappe4-autogain-hex.txt` mit Basis-SHA, Bau und
Lauf belegt (dort "339 geprueft" gegen heute 355). Ich habe das nicht geglaubt,
sondern die Zweikanalformel in Python eigenständig nachgerechnet: `modus_stereo`
-4,312390025805, `modus_left` und `modus_right` -2,670806897046, `modus_mid` und
`modus_side` -2,270777818370 — dieselben Zahlen, die B6 ausgibt und gegen die
Hexgoldens hält. Dieselbe Rechnung für den Monozweig gibt `left` und `mid`
-4,312390025805 (gleich dem Zweikanal-Stereowert, wie M-60 verlangt), `right`
und `side` exakt 0, und für M-65 (b) mono -144,890032333108 gegen zweikanal
-138,855681104606 — beides deckungsgleich mit der Ausgabe des Beins.

**Exakt +0,0 ohne Vorzeichenkorrektur.** M-57 und M-58 greifen den Zweig
`monoMittel == 1.0` (bei `right` bleibt `a_L` bitgenau `(1, 0)`, bei `side` ist
die Seitenkomponente bitgenau 0, `1.0 + (0,0)*h` bleibt `(1, 0)`; 121 Einsen
summieren sich exakt zu 121,0, geteilt durch 121 exakt 1,0). M-65 (a) greift den
Kurzschluss `:96`. In beiden Fällen kommt das positive Null direkt aus einem
`return 0.0`, nirgends steht ein `fabs`, ein `+ 0.0` oder eine
Vorzeichennachbesserung. Eigener Lauf: `signbit` 0 an allen drei Zeilen.
M-64 bestätigt, dass 0, 2 und 3 auf dem bitgleichen Zweikanalwert
`0xC0055DD001A5F001` bleiben. M-66 zählt im eigenen Lauf 4 Ableitungen außerhalb
und 0 im Audiopfad. M-62 und M-63 laufen am echten `SondeProcessor` über
`setBusesLayout` plus `prepareToPlay` (Mono, Stereo, Mono) und messen Kern UND
`dspBericht` nach jedem Wechsel, dazu die Bytegleichheit des Ausgangs mit und
ohne Auto-Gain und die unveränderten Statebytes.

## (b) R-311-5 — Berichtsgrenze ±120 dB (T3-15-09 Teil a, M-67 bis M-72)

**Geklemmt wird genau im Bericht und nur dort.** `berichtsAutoGainDb`
(`NakamaTransaktion.cpp:777-789`) prüft zuerst `std::isnan` und gibt `0.0`
zurück (positives Null, nicht `-0.0`), dann zwei Vergleiche gegen
`kBerichtAutoGainGrenzeDb`; jeder Wert innerhalb der Grenze fällt durch auf
`return roh` — kein `std::clamp`, keine Multiplikation, kein Runden, also auch
`-0,0` und das kleinste Subnormal bitgleich. ±∞ fallen ohne eigenen Zweig unter
die Vergleiche. Aufgerufen wird die Funktion an genau einer Stelle,
`aus.autoGainDb = berichtsAutoGainDb (prog->autoGainDb)` (`:822`), NACH dem
Programmbau; `prog` selbst bleibt unberührt, und die Liste `klemmungen` bekommt
weiterhin nur R7-Sidechain-Einträge (`:823-832`). `DspKern::autoGainDb()`
(`DspKern.h:312`) liest `autoGainBericht`, gefüllt aus `prog.autoGainDb`
(`DspKern.cpp:222`), und `autoGainLin` entsteht aus dem UNGEKLEMMTEN Wert
(`DspProgramm.cpp:339-340`) — die angewandte Rampe nimmt ihr Ziel von dort.
Eigener B3c-Lauf: Bericht -120,000000, Kern -199,767783, `autoGainLin`
0,000000000102709553 gleich 10^(dB/20), `klemmungen` 0.

**M-69 liest die Grenze aus dem geladenen Schema.** Der Fall holt
`schemaVar["$defs"]["dsp_bericht"]["properties"]["auto_gain_db"]["minimum"]`
und `["maximum"]` und hält `tx::kBerichtAutoGainGrenzeDb` dagegen
(`SchemaTestMain.cpp:1592-1600`) — keine zweite Zahl im Test. Zusätzlich prüft
er am Schema selbst, dass ±120 angenommen und 120,5 abgewiesen wird. Eigener
Lauf: Konstante 120,000000, Schema [-120,000000, 120,000000].

**Zweiter Weg auf den Draht: keiner.** `DspBericht` ist der einzige Träger von
`auto_gain_db` im Produktcode, `baueBericht` sein einziger Erzeuger, und
`SondeProcessor::dspBericht` (`:1298-1302`) sein einziger Zugang; einen
`state_report.dsp`-Sender gibt es im Sondenprozessor heute nicht
(`grep` über `eq-copilot/plugin` außerhalb von `tests/` findet keinen). Der
Broker führt `auto_gain_db` nirgends. Damit gibt es keinen Pfad, auf dem der
ungeklemmte Wert die Vertragsgrenze reißen könnte. Dass Bericht (-120) und
angewandter Ausgleich (-199,77) auseinanderliegen, ist die ausdrückliche Zusage
von M-71; der Deckel des angewandten Werts gehört zu Karte U54 und ist hier
kein Befund.

**Vertragsdiff.** Selbst gemessen: `git diff --numstat` gibt über
`eq-copilot/schemas/v3` genau `1 1` und über `eq-copilot/fixtures/v3` genau
`1 1`; über `eq-copilot/schemas/state` und `eq-copilot/identity` ist der Diff
leer. Die Schemadatei trägt 2395 CRLF und 0 reine LF, die geänderte
`$comment`-Zeile endet mit CR und ist reines ASCII; `MANIFEST.json` trägt
0 CRLF und 4231 LF. `minimum: -120` und `maximum: 120` stehen unverändert
(`eq-ipc-v3.schema.json:2327-2328`), `dsp_klemmung` ist nicht berührt. Die
Brokerzeile im Installer-Manifest ist vom Bauschritt geschrieben: der SHA-256 von
`broker/target/release/eqcop-broker.exe` ist
`F29F4AC43330F00FED43773A5D7AD69D9999F78A74B16CFA6FCF968D1B4F52AA` und damit
Zeichen für Zeichen der Wert in `nakama-installer-v1.json:52` — selbst
nachgerechnet. Der Erzeuger `erzeuge_v3_fixtures.py` und das Manifest tragen
denselben neuen `warum`-Text, und A8 bestätigt in meinem Lauf 406 bytegleiche
Dateien.

## (c) T3-15-11 (M-73 bis M-75) und F12 (R-311-4, M-76 bis M-80)

**Steuerrate: gemessen an der Stelle, die die Zusage trägt, ohne Verhaltensänderung.**
Der Änderungssatz fasst für T3-15-11 nur den Absichtskommentar an
(`DspProgramm.h:72-90`); `kDynamikSchritt` bleibt 8, und kein anderer Produktcode
dieser Zeile ändert sich. Der Fall (`DspGoldenTestMain.cpp:2622-2728`) vergleicht
den Prüfling gegen einen Referenzkern, der sich in EINEM Wert unterscheidet
(Range 0, derselbe SVF-Weg), misst den Tap `post_committed` Sample für Sample mit
`memcmp` und liest dazu am Bandzustand `schrittRest`, `svfVon` und `svfNach`. Die
volle Wirkung wird nicht behauptet, sondern gezeigt: `svfVon` bei `d + 8` ist per
`memcmp` genau der Satz, der bei `d` entworfen wurde. Eigener Lauf, alle drei
Raten: erste Abweichung exakt bei `d + 1` für p = 0, 1, 3, 7; erste Wirkung 1 bis
8, volle 8 bis 15 Samples; 0,167 bis 0,3125 ms bei 48 kHz, 0,181 bis 0,340 ms bei
44,1 kHz, 0,083 bis 0,156 ms bei 96 kHz — dieselben Zahlen, die der berichtigte
Kommentar und die Matrix nennen.

**Recall: Toleranz, Geltungsbereich, Referenzbeginn und t_E stammen aus F-12 und
sind nicht stillschweigend aufgeweicht.** `nak311TeE`
(`TransactionTestMain.cpp:2860-2865`) schreibt
`t_E = max (kFadeSamples, 5*tau_a*fs + 10*Q*A_max*fs/(pi*f0) + 16)` mit
`A_max = 10^(max(0, g0, g0 + Range)/40)` eigenständig aus, ruft also keine
Produktfunktion als Orakel. Die Schranken stehen fest im Code: `0.1` dB ab t_E
und `|g0| + |Range| + 0.1` dB davor (`:3053-3054`); nichts davon ist eine
gemessene Zahl mit Reserve. Referenzbeginn ist das erste Sample des ersten Blocks
der geladenen beziehungsweise neu vorbereiteten Instanz, die Vergleichsinstanz
läuft ununterbrochen weiter und bekommt ab dort denselben Eingang
(`:2916-2947`). Die drei Prüflinge und beide Threshold-Lagen entsprechen der
Matrix. Eigene Messung: t_E 345,577491 / 2721,577491 / 120321,577491 Samples für
Prüfling 1 (Attack 0,1 / 10 / 500 ms), 256,000000 für Prüfling 2, 345,577491 für
Prüfling 3 — Sample für Sample die Zahlen aus M-77. Die größte Abweichung ab t_E
über alle zwölf Vergleiche ist 0,029019 dB gegen die Schranke 0,1 dB; davor
höchstens 12,000001 dB gegen 12,100000 dB. M-76 ist in meinem Lauf in Ausgang
UND Tap bitgleich zum ununterbrochenen Kern, M-79 zeigt 4708 Bytes vor und nach
zwei Sekunden eingeschwungenem Audio bei einer Auslenkung von -12 dB und
Revision 1 — die Hüllkurve hat sich also nachweislich bewegt.

## (d) Rotbeweise — fällt jede Zeile an der Zusage?

Ja, für alle 24 Zeilen (25 Dateien, M-65 in zwei Hälften). Ich habe die
Mutationsstellen am Code des ZIEL nachgemessen: die in den Rohdateien
angegebenen `SHA-256 vorher` stimmen für alle sieben berührten Quelldateien mit
den heutigen Dateien überein —
`DspProgramm.cpp 9C0AEDA7…`, `DspKern.cpp 29DD414C…`, `DspFilter.h C3C76C17…`,
`NakamaTransaktion.cpp 11F8D3B0…`, `NakamaTransaktion.h E2C0C868…`,
`SondeProcessor.cpp F81424034…`, `eq-ipc-v3.schema.json 30B68F20…`. Einzige
Abweichung ist M-63 mit `1FB741EB…` für `NakamaTransaktion.cpp`; das ist
nachweislich der Stand des Teil-a-Commits `48f50eca`, an dem diese Teil-a-Zeile
gemessen wurde (`git show 48f50eca:… | sha256sum` gibt genau diesen Wert). Jede
Datei trägt `SHA-256 nachher` gleich `vorher`, `git status --short` über
`eq-copilot` und `tools` ist leer — keine Mutation blieb stehen.

Die Stellen selbst tragen die jeweilige Zusage, nicht einen Nebeneffekt:
`DspProgramm.cpp:115` ist die Zweigbedingung `if (p.kanaele == 1)` (M-57, M-58,
M-60, M-61, M-64, M-65 (b); die Mutationsstelle an der Zweigbedingung ist vom
Dirigenten angenommen), `:123-153` der Kaskadenkörper (M-59), `:96` und `:162`
die beiden Nullzusagen (M-65 (a)), `:203` die Rückgabe der Ableitung selbst
(M-71 — Klemmung in der Ableitung statt im Bericht). `SondeProcessor.cpp:270`
ist die Weitergabe an den Transaktionskern (M-62), `NakamaTransaktion.cpp:356`
die Übernahme im Transaktionskern (M-63), `:822` die Klemmung im Bericht
(M-67), `:785` der NaN-Zweig (M-70), `NakamaTransaktion.h:408` die Konstante
`kBerichtAutoGainGrenzeDb = 120.0` (M-68, M-69),
`eq-ipc-v3.schema.json:2328` das `maximum` (M-72). Im Kern:
`DspKern.cpp:464` der Übernahmepunkt in `blockrand` (M-66), `:712` das
Mischgewicht `tSchritt` (M-73 bis M-75), `:275` die Pfadkennung (M-76),
`:547` die Fadelänge (M-77 Crossfadeterm), `DspFilter.h:284` der Attackpol
(M-77 Hüllkurventerm), `:227` die Detektorbandbreite (M-77 Frequenzterm) —
alle drei Teile von M-77 und M-78 je mit eigener Mutation und eigener
Fallzeile. M-79 mutiert den Statepfad (`SondeProcessor.cpp:1007 ff.`), M-80
den Ladeweg (`:1064`). Jede Rohdatei trägt die Zeile "Die Zusage faellt an der
erwarteten Pruefung: JA" samt der gefallenen Prüfung und den grün gebliebenen
Nachbarn.

## (e) Grenzen

**Etappen 2 und 3 bleiben grün.** Eigener Lauf: B6 355 geprüft / 0 Fehler,
A16 114 / 0, B7 187 / 0, B3c 157 / 0. Die Zeilen 311/M-01, 311/M-10 bis
311/M-55 und 311/M-90 bis 311/M-95 stehen in meinen Ausgaben durchgehend auf
`ok`.

**M-82 und M-83 selbst nachgemessen.** Ich habe die Abschnitte C, G und H meines
eigenen B6-Laufs gegen die im Rohbeleg wörtlich gespeicherten BASISZEILEN des
Basis-SHA gediffnt: 106 Zeilen, genau EINE Abweichung — der bestehende Fall
`auto_gain_wird_nicht_im_audiothread_gerechnet` trägt den M-66-Zusatz und zählt
4 statt 2 Ableitungen außerhalb, während der gemessene Wert
`ableitungenImAudiopfad` 0 bleibt. Genau diese Ausnahme nennt M-83; jede andere
Zeile der RBJ- und Stereo-Goldens ist bytegleich zur Basis.

**Kanalzahl erreicht weder State noch Draht noch `state_hash`.** `kanaele` und
`kanalzahl` kommen im ganzen Produktcode an genau den Stellen vor, die der
Änderungssatz anlegt (Programmfeld, Kernmember, Transaktionskernmember, drei
Signaturen, zwei Lesestellen); es gibt keinen Serialisierer, kein Schemafeld und
keinen Kanonisierungspfad dafür. `aus.jcs` und `aus.hash` in `baueBericht`
stammen aus `param::dtoKanon` über den bestätigten Zustand, nicht aus dem
Programm — die Klemmung kann den Hash nicht bewegen. M-63 misst es am echten
Prozessor: 4708 Bytes vor und nach drei Layoutwechseln gleich, Revision
unverändert, 0 Host-Dirty-Meldungen. Keine neue Stateversion, `schemas/state`
unberührt, A14 grün.

**Vertrag, Identität, Audio-Thread.** Der v3-Diff ist die eine `$comment`-Zeile;
`minimum`, `maximum`, Discriminator, Zieladresse, Revision und Capability sind
unverändert (der ganze Dateidiff ist eine Zeile). `eq-copilot/identity` ist leer
im Diff, B1 grün (120 / 0). Im Audiopfad: `RtWache::meldeAbleitung()` steht am
Anfang von `leiteAutoGainAb` (`:89`), also VOR dem Monozweig — der neue Zweig
wird mitgezählt; B6 misst 0 Ableitungen im Audiopfad und im
Allokationslauf über 4000 Blöcke 0 Allokationen und 0 Sperren. Die Monokaskade
läuft ausschließlich im Worker (`baueVor`) beziehungsweise auf dem
Nachrichtenthread (`baueBericht`), beides außerhalb des Callbacks, und
allokiert nicht.

## Befunde

### DEFEKT

Keiner.

### LÜCKE

**L-1 — `releaseResources` setzt die Kanalzahl nur im Kern zurück.**
`DspKern::freigeben` setzt `kanalzahl = 2` (`DspKern.cpp:140`), der
Transaktionskern behält seinen Wert: `releaseResources`
(`SondeProcessor.cpp:282-296`) ruft `setzeSamplerate` nicht. Ein `dspBericht`
zwischen `releaseResources` und dem nächsten `prepareToPlay` baut sein Programm
deshalb mit der zuletzt vorbereiteten Kanalzahl — und mit der zuletzt
vorbereiteten Samplerate, denn `fs` bleibt aus demselben Grund stehen. Der
Kommentar an `DspKern.h:110-122` sagt "`freigeben` setzt beide zurueck, damit
keine veraltete Kanalzahl einen Programmbau ohne neue Vorbereitung erreicht";
für die zweite Lesestelle gilt das nicht. Kein Widerspruch zwischen Kern und
Bericht (beide tragen dieselbe alte Zahl, und der Kern rechnet in diesem Fenster
gar nicht), und die Kanalzahl folgt exakt dem Muster, das die Samplerate schon
vor der Etappe hatte. Gate, Matrix und Entwurf sagen zu dem Fenster nichts.

**L-2 — `prepareToPlay` gibt die Kanalzahl nur im Gutfall weiter.** Kern und
Transaktionskern bekommen die gemessene Zahl nur innerhalb von
`if (sichereRate > 0.0 && maxBlock > 0)` (`SondeProcessor.cpp:258`). Bei
`maxBlock == 0` oder einer vom Riegel auf 0 gesetzten Samplerate behalten beide
ihren vorherigen Stand — wieder gemeinsam, also ohne Divergenz, und wieder wie
die Samplerate seit jeher. Keine Matrixzeile deckt es ab.

### HÄRTUNG

**H-1 — Vorgabeargument `kanaele = 2` an `Transaktionskern::setzeSamplerate`.**
Die Vorgabe hält heute jeden Aufrufer bitgleich (F-10), macht aber einen
künftigen Aufruf `setzeSamplerate (rate)` zu einem stillen Rücksetzen der
Kanalzahl auf 2, während der Kern seine behält — genau die Divergenz, die M-62
verhindert. Ein eigener Setzer oder eine Signatur ohne Vorgabe nähme die Falle
weg. Von keiner Zusage verlangt.

**H-2 — die Monoregel hängt an der Reihenfolge M/S-Stufe vor den Bändern.**
Dass ein `right`-Band im Monobus den geschriebenen Kanal nicht berührt, gilt nur,
weil Width und Mono-Bass VOR den acht Bändern laufen (`DspKern.cpp:797-837`).
Liefe die M/S-Stufe hinter den Bändern, führte sie die rechte Komponente nach
links zurück und M-57 fiele an seiner Audiohälfte. Die Reihenfolge ist als
allgemeine Regel eingefroren (§3.0, §5.3), aber keine Zeile bindet sie an
R-311-3.

**H-3 — `getTotalNumInputChannels()` statt der Zahl des Hauptbusses.**
Heute deckungsgleich, weil der Prozessor genau einen Eingangsbus deklariert
(`SondeProcessor.cpp:133-135`). Käme je ein Sidechain-Eingang hinzu, zählte
`getTotalNumInputChannels()` ihn mit, und ein Monohauptbus mit Monosidechain
wählte still die Zweikanalformel. `getChannelCountOfBus (true, 0)` wäre gegen
diese Entwicklung dicht.

**H-4 — Behauptung B6, "zwei bis acht Baender".** Gemessen sind ein Band
(M-60), zwei Bänder (M-59 und das Selbstaudit) und acht Bänder (M-65 (b));
drei bis sieben Bänder misst kein Fall. "zwei und acht" wäre die Zahl, die der
Messung entspricht.

## Geprüft

Selbst an der Quelle gelesen und gegengerechnet: der vollständige Diff aller
16 Auftragsdateien; `leiteAutoGainAb`, `baueProgramm`, `DspKern::bereiteVor`,
`freigeben`, `baueVor`, `publiziereVorbau`, `verarbeiteBand`, `verarbeiteBank`,
`verarbeitePfad`, `verarbeite`, `verarbeiteStueck`; `SondeProcessor::prepareToPlay`,
`releaseResources`, `reset`, `processBlock`, `dspBericht`, die Busdeklaration;
`Transaktionskern::setzeSamplerate`, `berichtsAutoGainDb`, `baueBericht`; die
RBJ-Entwürfe in `DspFilter.h`; die neuen Fälle in `DspGoldenTestMain.cpp`
(Abschnitte H2 und F), `SondeNullTestMain.cpp` (Layoutabschnitt),
`SchemaTestMain.cpp` (Berichtsgrenze) und `TransactionTestMain.cpp`
(Abschnitt T); die vier geänderten Behauptungen in `tools/beweise.ps1`.

Selbst gefahren und gemessen: B6, A16, B3c, B7, B1, A14, A8 und A5 am Zielstand,
alle Exit 0 ohne harte Fehlerzeile; der Zeilenvergleich meiner eigenen B6-
Abschnitte C, G und H gegen die gespeicherten Basiszeilen; die SHA-256 aller
sieben mutierten Quelldateien gegen die Rotbeweise; der SHA-256 des
Brokerbinarys gegen das Installer-Manifest; die Zeilenenden und die
ASCII-Reinheit der Schema- und Manifestzeile; `git diff --numstat` über
`schemas/v3`, `fixtures/v3`, `schemas/state` und `identity`; der Textvergleich
des Zweikanalzweigs Basis gegen Ziel; die Binärzeitstempel gegen die
Quellzeitstempel. Zusätzlich habe ich die Zweikanal- und die Monoformel in
Python unabhängig nachgerechnet und die vierzehn Hexgolden-Werte, die
M-60-Gleichheit, M-65 (b) sowie -199,77 dB und -92,29 dB reproduziert.

Nur gelesen, nicht selbst nachgemessen: der Wortlaut der Rotbeweis-Rohdateien
(ihre Läufe sind vom 20.09.2026 und nicht wiederholbar, ohne den Prüfbereich zu
verändern — geprüft habe ich davon die Mutationsstellen und die Quell-Hashes);
das Manifest `docs/beweise/NAK-311.md` als Referenz für Matrix, Regeln,
Zusagen, Bauplan und Feinheiten; der Erzeugerbeleg des Hexgoldens (dessen
Zahlen ich unabhängig nachgerechnet habe).

## Nicht geprüft

Nicht Prüfgegenstand nach Auftrag und deshalb hier ohne Urteil: der volle Kanon
(die 69 Beine; ich habe acht davon einzeln gefahren), der Laufzeit-Arm,
"Nulltest im Host bitidentisch", die Gate-Sätze zu T3-15-07, T3-15-06,
T3-15-10, F08 und T3-15-09 Teil b (Etappe 5, Karten U44 bis U47 und U54),
alles unter `docs/**`, die im Auftrag §2 aufgezählten datierten oder
vorgemerkten Härtungen und Abweichungen, `tools/dirigent/pruefliste.md` sowie
die Sicherheitsbefunde T3-12-* aus S25i. Gebaut habe ich nichts; ich habe
ausschließlich die vorhandenen Binaries vom 20.09.2026 03:54 gefahren und keine
Repo-Datei außer diesem Bericht geschrieben.

HEAD am Ende: `c5c00591b2e67a843f3b0cf891290b0a505782f3` — unverändert
gegenüber dem Start.
