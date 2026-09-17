# Tiefenaudit 3 · Phase 15 · Gegenprüfung Paket 2 (Skeptiker)

> **Rolle dieses Berichts:** Widerlegungsprüfung zu Paket 2 im Tiefenaudit 3, Phase 15
> (17.09.2026, Quellstand `e008811efa7650f3e0cc2332cc21f9e268cdd125`). Ein anderer
> lesender Prüfer als der Finder hat jeden Kandidaten gezielt zu widerlegen versucht und
> dabei eigene Nachrechnungen der aus der Quelle abgeschriebenen Formeln angestellt (keine
> Produktausführung). Sein Urteil ist für Kategorie und Schwere verbindlich, soweit
> [phase-15-zentral.md](phase-15-zentral.md) nichts anderes begründet; die Schwere von K1
> hat der Dirigent dort ausdrücklich angehoben. Kandidat `K1` bis `K7` entspricht
> `P15-D-01` bis `P15-D-07` in
> [phase-15-paket2-dsp-numerik.md](phase-15-paket2-dsp-numerik.md).


Datum 17.09.2026. Quellstand HEAD `e008811efa7650f3e0cc2332cc21f9e268cdd125` (vor und nach
der Prüfung gleich; `git status --short` unverändert: `docs/audits/2026-09-15-tiefenaudit/BEFUNDE.md`
modifiziert, `briefing-hub/`, `nimbalyst-local/`, `roh/phase-14-blindspots.md` untracked).
Rein lesend. Keine Repo-Datei geändert, kein Bau, kein Test, kein Kanon, keine Installation,
kein FL-/MCP-Aufruf. Geschrieben wurde ausschließlich im Scratchpad.

**Auftrag:** jeden der sieben Kandidaten zu widerlegen versuchen. Die Kandidaten galten als
unbewiesene Behauptungen; jede Quellstelle unten habe ich selbst gelesen, jede Zahl selbst
nachgerechnet (eigene Skripte `p15-gegen2-nachrechnung.py`, `p15-gegen2-alternativen.py`;
**Nachrechnung abgeschriebener Formeln, keine Produktausführung**). Die Skripte des Finders
habe ich gelesen, aber keine ihrer Zahlen übernommen.

**Ergebnis:** 4 BESTÄTIGT, 3 HERABGESTUFT, 0 WIDERLEGT, 0 DUPLIKAT, 0 UNENTSCHIEDEN.

---

## K1 — Pfadrampen überdauern `eq_enabled` aus/ein

**URTEIL: BESTÄTIGT** · Kategorie **DEFEKT** · Schwere **medium** (Klasse K4)

**Stärkstes gefundenes Gegenargument (hält nicht):** Die zitierte Zusage trägt nicht.
M-07 (`docs/beweise/SONDE-015.md:1040`) nennt wörtlich „alle **Filter- und
Hüllkurvenzustände**"; eine Rampe ist keins von beiden, sie gehört nach E-16
(`:3409`) dem Pfad. R2 Feinheit 4 (`:1573`) sagt zwar „alle Zustände auf 0", verweist
aber im selben Satz auf M-07, und „Rampe auf 0" wäre Stille — die Lesart ist nicht
haltbar. Zweitens hält das ausdrückliche Rotbeweis-Kriterium von M-03: **am
Umschaltsample** gibt es keinen Sprung (Fadesample 0 trägt die Quelle allein,
`DspKern.cpp:798-804`), der Ausgang endet nach 256 Samples bitgenau auf dem Ziel, und der
Kommentar `DspKern.cpp:383-384` entscheidet das Rampenverhalten für die **ausblendende**
Bank ausdrücklich so.

**Warum es trotzdem bricht:** Zwei andere Zusagen greifen, und beide sind wörtlich.
1. **M-03** (`:1036`) sagt „Der Wechsel ist **klickfrei**". Das Maß dafür hat das Projekt
   selbst festgelegt (E-31, `:3425`) und in B6 gebaut: neben dem Umschaltsample eine
   **Wache über den ganzen Lauf** (`groessterSprung` gegen `kaltSchranke`,
   `eq-copilot/plugin/tests/DspGoldenTestMain.cpp:1075-1083`). In dieser Folge sind Quell-
   und Zielwert identisch (Dry gegen neutralen Kern), die Fadeschrittweite also 0, die
   Kaltschranke ≈ 0 — gemessen habe ich einen Nachbarsprung von **0,0575 je Sample** bei
   Eingang 1,0. Das reißt die projekteigene Klickschranke um Größenordnungen.
2. **Grundgesetz** (`CLAUDE.md`): „Verarbeitung nur, wenn der User sie einschaltet."
   Angewandt wird für 256 Samples ein Gain, der weder im bestätigten noch im wirksamen
   Zustand steht (`Transaktionskern::wirksam`, `NakamaTransaktion.cpp:340-348`): der User
   hat +24 dB bereits zurückgenommen.

**Geprüfte Stellen (selbst gelesen):**
`eq-copilot/plugin/dsp/DspKern.h:310-336` (`setzeSofort`/`setzeZiel`/`tick`/`ruhtBei`),
`:339-344`, `:355-365`; `DspKern.cpp:85-93` (`ruhe()` — einzige Rückstellung auf 1,0),
`:110` und `:138` (die einzigen zwei Aufrufer: `bereiteVor`, `freigeben`),
`:150-194` mit `:182-185` (`beendeAudiohistorie` setzt `setzeSofort (r->ziel)`),
`:357-395` (`blockrand`; `:366` laufender Übergang, `:374-379` ENDE-Marke mit
`if (alt < 0) return;`, `:383-384`), `:398-424` (`setzeZiel` beim Einschalten),
`:636-641`, `:651-662`, `:709-732` (Anwendung je Sample), `:744-768`, `:775-814`
(Crossfade aus der Ruhe, `von = dry`), `:818-824`;
`eq-copilot/plugin/sonde/SondeProcessor.cpp:143-165` (112 Hostparameter registriert,
`setzeSamplegenaueAutomation (false)`), `:232-269` (`prepareToPlay` ruft `bereiteVor`),
`:287-309` (`reset` ruft nur `beendeAudiohistorie`), `:1296-1304`, `:1413-1443`;
`eq-copilot/plugin/state/NakamaTransaktion.cpp:340-348`, `:753-763`;
`eq-copilot/schemas/state/nakama-parameter-v2.json` (`output_trim_db` ±24 dB, `rampe`,
`host_parameter` true; `eq_enabled` blockrand, `topologisch` false);
`DspGoldenTestMain.cpp:1089-1117` (M-07-Test), `:1040-1087` (B-5/W-4-Wache);
`docs/beweise/SONDE-015.md:1036`, `:1040`, `:1055`, `:1079`, `:1573`, `:3409`, `:3425`.

**Nachrechnungsprüfung:** Abbildung des Codes korrekt. Ich habe die Folge **blockweise**
nachgerechnet (Blockgrößen 1, 64, 300 — das Ergebnis ist blockunabhängig, weil Rampenrest
und Fadeschritt samplegenau im Zustand liegen): max Ausgang/Eingang **4,6832 = +13,41 dB
bei Sample 128**, Endwert exakt 1,000000, größter Nachbarsprung 0,0575. Gegenproben, die
der Finder nicht nennt: ohne Trimwechsel (der B6-M-07-Fall) bleibt das Maximum exakt
1,000000; die Gegenrichtung (−24 dB alt → 0 dB neu) erzeugt nur eine Delle von −2,30 dB.
Die Behauptung ist damit auf den Fall „alter Trim über dem neuen" eingegrenzt, dort aber
bestätigt.

**Erreichbarkeit — der schärfste Punkt:** **einziger Kandidat des Pakets, der heute ohne Gen
und ohne belegtes Band erreichbar ist.** `eq_enabled` und `output_trim_db` sind
Hostparameter, nicht topologisch, und gelangen auch bei abgeschalteter samplegenauer
Automation ins Programm — derselbe Weg, den T3-05-02 unabhängig belegt
(`BEFUNDE.md:412`). Es genügt FL-Automation oder das generische Plugin-Fenster.
`reset()` (FL-Transportstopp) **heilt es nicht**, sondern konserviert den Altstand
(`setzeSofort (r->ziel)`); nur `prepareToPlay` setzt zurück.

**Kein Duplikat:** T3-01-09 (VST3-Hostbypass, anderer Eintritt), T3-01-05 (Zusammenfallen
von Hostwerten im 5-ms-Takt — hier braucht es im Gegenteil getrennte Takte), T3-14-02
(Bandzustand, nicht Pfadrampen), NAK-81 (Planarbeit zu Gain In/Out).

**Zur Schwere:** medium ist vertretbar (Dauer 5,8 ms bei 44,1 kHz, klar begrenzt, Folge
aus drei Schritten). Für **high** spräche, dass es der einzige heute per FL-Handgriff
erreichbare Fall ist und +13 dB auf einem Bus hörbar knallen; die Entscheidung überlasse
ich dem Dirigenten.

---

## K2 — Detektor mit Hold/asymmetrischer Ballistik liegt über dem RMS

**URTEIL: HERABGESTUFT** · Kategorie **LÜCKE** · Schwere **low** (vorgeschlagen war LÜCKE
medium, der Finder schlug DEFEKT vor) · Klasse K3 mit K5-Anteil

**Stärkstes gefundenes Gegenargument (hält):** Der gebaute Detektor **ist wörtlich das,
was R7 Feinheit 3 vorschreibt** (`SONDE-015.md:1711`): „quadratischer Mittelwert über eine
Ein-Pol-Hüllkurve mit den Koeffizienten `exp(−1/(fs·τ))` und **getrenntem Hold-Zähler**".
M-26 (`:1069`) beschreibt die drei Stufen genau in der gebauten Reihenfolge. Und M-18
(`:1061`) definiert `e_db` **selbstbezüglich** als „der RMS-Pegel **des Detektors nach
Attack/Hold/Release**" — nicht als den RMS des Eingangssignals. Der Versatz gegen den
wahren RMS ist eine unvermeidliche Eigenschaft **jeder** Ballistik auf der Momentanleistung;
eine Umsetzung, die R7 Feinheit 3 erfüllt und den Versatz vermeidet, gibt es nicht.
**Damit ist die DEFEKT-Lesart widerlegt.**

**Die M-19-Teilbehauptung widerlege ich ausdrücklich:** Der Finder leitet ab, ein Signal mit
RMS 2 dB unter Threshold erzeuge −0,76 dB statt exakt 0,0 und breche M-19. M-19 (`:1062`)
spricht von „Pegel unter Threshold", und der Pegel der Kennlinie ist nach M-18 `e_db`, also
der Detektorpegel. Der Code liefert für `e_db ≤ t` bitgenau 0,0 (`DspFilter.h:355-361`:
`if (! (ueber > 0.0)) return 0.0;`). **M-19 ist nicht verletzt.**

**Was übrig bleibt (deshalb LÜCKE, nicht WIDERLEGT):** Es gibt keine Regel und keinen
datierten Entscheid dazu, **gegen welchen Pegelbegriff** der vom User eingestellte
`threshold_db` wirkt. Mit Hold ≥ einer halben Tonperiode wird der Detektor rechnerisch zum
**Spitzenleistungs**-Detektor; derselbe Threshold greift dann bei tonalem Material 3 dB und
bei rauschartigem Material rund 8,7 dB früher als eine RMS-Lesart erwarten ließe. Dazu ein
echter Beweisschatten: **alle** Knie- und Zeitmessungen laufen mit einem Quadraturton
konstanter Momentanleistung, bei dem der Versatz konstruktionsbedingt exakt 0 ist.

**Geprüfte Stellen (selbst gelesen):** `DspFilter.h:258-265` (Kommentar „das wäre kein
RMS"), `:266-288`, `:290-329` (`HuellkurveZustand::tick`: Vergleich gegen die
**Momentan**leistung, Hold-Neuladung bei jedem Anstieg), `:331-341`, `:343-361`;
`DspKern.cpp:520-554` (Detektorlauf, `leistungEin = (d0²+d1²)·0,5`, E-5),
`:557-581` (Kennlinie auf `leistungInDb (z.huelle.leistung)`), `:604-612`;
`DspProgramm.cpp:240-260`; `DspGoldenTestMain.cpp:425-447` (`fahreStereoTon`: L = sin,
R = cos — konstante Leistung), `:1657-1689` (Plateau, pegelunempfindlich),
`:1691-1728` (Knie, Quadraturton, A = R = 500 ms), `:1894-1970` (Sprungantwort,
Quadraturton); `eq-copilot/schemas/v3/flatbuffers/` (`band_dynamic_gain_db` ist die
**Auslenkung**, kein Pegelfeld — kein Verbraucher sieht `e_db`);
`SONDE-015.md:1061-1062`, `:1069`, `:1696-1697`, `:1711`, `:3419` (E-25).

**Nachrechnungsprüfung:** Abbildung korrekt, Zahlen im Kern bestätigt, eine Zahl
zurückgestuft. Eigene Messung (1 kHz, Bandpass Q 0,707, L = R):
A/H/R 10/0/100 ms **+2,12 dB**; 1/0/20 ms +2,42 dB; Hold ≥ halbe Periode **+3,01 dB**
(exakt der theoretische Spitzen-/Effektivwertabstand); A = R, Hold 0 **−0,01 dB**;
Quadraturton in jeder Einstellung **+0,00 dB**. Rauschen (L = R): 10/0/100 ms +4,18 dB,
10/30/100 ms **+8,66 dB**, 1/0/300 ms +8,31 dB — die Angabe „bis +9 dB" ist damit knapp
optimistisch, der Befundkern bleibt.

**Erreichbarkeit:** nur mit belegtem dynamischem Band, also heute über Transaktion oder
geladenen Zustand (NAK-306, `docs/offene-punkte.md:314`), danach Hostautomation von
threshold/range/attack/hold/release.

**Warum low:** keine Zusage verletzt, kein Verbraucher des Pegels auf dem Draht oder in der
Fläche, die gemeldete Auslenkung bleibt gemessen und ehrlich (M-27), und die gebaute Form
ist verbreitete Praxis. Was fehlt, ist ein Entscheid plus ein Testfall mit schwankender
Leistung.

---

## K3 — Rampenübergang statischer Bänder mit großen Frequenzsprüngen

**URTEIL: BESTÄTIGT** (mit verschärfter Beweislage) · Kategorie **LÜCKE** · Schwere
**medium** · Klasse K3 mit K5-Anteil

**Stärkstes gefundenes Gegenargument (hält für die Kategorie, nicht für den Befund):**
Keine Zusage ist wörtlich verletzt. §44.5 (`Entwurf:3109`) bindet die −100 dBFS
ausdrücklich an den „**definierten** Ramp-Test", §44.2 (`:2990-2991`) verlangt nur „Stetige
Parameter werden geglättet" — beides ist erfüllt. E-19 (`SONDE-015.md:3412`) begründet die
lineare Mischung allein mit **Stabilität**, und dieses Argument stimmt: ich habe die
Zwischenentwürfe geprüft, alle Polradien liegen unter 1 (0,675 → 0,996), der größte
Betragsgang eines Zwischenentwurfs ist +9,73 dB. Also LÜCKE, nicht DEFEKT.

**Warum der Befund trotzdem steht — und härter als vorgelegt:** Ich habe die vom Finder
nicht gerechnete Gegenprobe gemacht, ob das Verfahren oder die Physik den Ausschlag
erzeugt:

| Fall (48 kHz, 1-kHz-Sinus 0,5) | Rampe 256 (Produkt) | Sofortsprung | Rampe 4096 | topologischer Crossfade 256 | Neuentwurf je Sample über 256 |
| --- | --- | --- | --- | --- | --- |
| LowShelf +6 dB Q 0,707, 5 kHz → 50 Hz | **+17,41 dB** | +6,85 dB | +3,74 dB | −0,22 dB | +1,29 dB |
| LowCut Q 0,707, 2 kHz → 20 Hz | **+22,20 dB** | +22,70 dB | +4,79 dB | +0,13 dB | +1,09 dB |
| LowCut Q 0,707, 20 Hz → 2 kHz | +2,08 dB | −12,35 dB | +4,26 dB | −0,45 dB | −0,74 dB |

Der Produktweg ist im Shelf-Fall **schlechter als ein Sofortsprung** und um 16 dB
schlechter als ein parameterstetiger Neuentwurf über dieselben 256 Samples. Der Mechanismus,
den der Kern für topologische Wechsel bereits besitzt, löst denselben Sprung mit ≈ 0 dB.
Zusätzlich fand ich einen praxisnäheren Fall als den vorgelegten: der Sweep eines Low-Cut
nach unten (+22,2 dB), eine gewöhnliche Produktionsbewegung.

**Wichtige Eingrenzung, die den Befund ehrlich hält (selbst gerechnet):** Ein **stufenweiser**
Sweep ist harmlos. 2 kHz → 20 Hz in 20 Stufen zu je einer 256-Sample-Rampe: **+0,02 dB**;
in 6 Stufen: +0,28 dB; in **einer** Stufe: +22,20 dB. Gefährlich ist also allein der
**einzelne große Sprung** — Presetladen, Undo/Redo, apply mit gleicher Topologie, ein
getippter Wert, eine Automationsstufe. Genau diese Wege sind gebaut
(`NakamaTransaktion.cpp:519-574`).

**Geprüfte Stellen (selbst gelesen):** `DspKern.cpp:39-52` (`mische` mit E-19-Begründung),
`:398-414` (Zustand wandert mit, `rest = kRampeSamples`), `:459-502` (t je Sample:
`t = 1 − (rampeRest − i − 1)/256`, über Blockgrenzen stetig), `:586-591` (statischer Weg),
`:780` (Quellbank nur als Koeffizientenquelle); `DspFilter.h:90-96` (DF2T), `:133-149`
(Low-Shelf), `:187-201` (Low-Cut); `DspProgramm.cpp:139-159` (`rampenKompatibel` kennt
**keine** Wertegrenze); `DspSvf.h:12-19` (eigene Begründung: RBJ-Zustand ist beim
Koeffizientenwechsel nicht stetig — „genau das erzeugt das Knacksen");
`DspGoldenTestMain.cpp:1482-1552` (Referenz **ist** derselbe DF2T mit derselben linearen
Mischung, `:1528-1545`; geprüft nur Bell, gain 3→9, freq 1000→1400, Q 1→3);
`Entwurf:2990-2991`, `:3109`; `SONDE-015.md:1055`, `:1725-1726`, `:3412`.

**Nachrechnungsprüfung:** Abbildung korrekt (t-Zählung, Zustandsübernahme, Reihenfolge,
Blockteilung eigens geprüft). Spitze 7,3929 bei Sample 444 — also **nach** dem Rampenende
(256). Der Ausschlag ist damit nicht die Resonanz eines Zwischenentwurfs, sondern das
Ausklingen der angeregten tiefen Polstelle (Radius 0,996, Zeitkonstante ≈ 5,3 ms); die
Deutung des Finders („zeitvariant, weil die Bedeutung der DF2T-Zustände an den Koeffizienten
hängt") trägt.

**Kein Duplikat:** T3-14-02 sitzt an derselben Stelle (`DspKern.cpp:397-408`), betrifft aber
den Lebenszyklus, nicht die Koeffizientenbahn. Keine Registerzeile.

---

## K4 — Topologiewechsel an einem Slot startet alle Bänder kalt

**URTEIL: BESTÄTIGT** · Kategorie **LÜCKE** · Schwere **medium** · Klasse K4

**Stärkstes gefundenes Gegenargument (hält für die Kategorie):** Der Kaltstart ist
**ausdrücklich entschieden** (E-8, `SONDE-015.md:3400`: „Nein — sie startet kalt, alle
Zustände auf 0"), und der Crossfade zwischen zwei vollständigen Programmen ist R8- und
§44.2-Wortlaut. Für den **gewechselten** Slot ist der Kaltstart sogar richtig — die
Gegenrichtung ist als T3-14-02 bereits ein Defekt. Es gibt keine Zusage zur Historie
**unveränderter** Slots. Also LÜCKE, nicht DEFEKT.

**Was die Begründung von E-8 nicht trägt (beides selbst geprüft):**
1. „Der Crossfade über 256 Samples deckt die Transiente ab" — 256 Samples sind 5,3 ms; ich
   messe Einschwingzeiten von 20 bis 600 ms.
2. „ein übertragener Zustand wäre ein geteilter Zustand zwischen zwei Bänken, und §44.2
   verbietet das ausdrücklich" — §44.2 verbietet Teilen **zwischen beiden Pfaden**
   (`SONDE-015.md:228-229`, M-46 `:1099`), nicht zwischen zwei Bänken desselben Pfades;
   E-19 überträgt dort genau so (`DspKern.cpp:400-408`). Die Begründung stützt die
   Entscheidung also nicht.

**Geprüfte Stellen (selbst gelesen):** `DspKern.cpp:237-276` (`publiziereVorbau`, `:266-271`
kalte Bank), `:398-414` (Zustandsübernahme nur bei `nurRampen`), `:796-813`;
`DspProgramm.cpp:148-157` (jede Abweichung **irgendeines** aktiven Slots macht den ganzen
Wechsel topologisch); `DspBankPool.cpp:8-18` und `:38-42` (`BandZustand::nullen` nullt
statisch, SVF, Detektor, **Hüllkurve**, `schrittRest` und `auslenkungDb`);
`DspBankPool.h:58-78`; `NakamaTransaktion.cpp:340-348` (`wirksam()` lässt topologische
Hostwerte fallen — `enabled` trägt im Vertrag aber `topologisch: false` und kommt deshalb
durch); `nakama-parameter-v2.json` (`enabled`: bool, blockrand, `topologisch` false,
`host_parameter` true); `DspGoldenTestMain.cpp:1778-1828` (M-25, ein Band, zweite
Differenz), `:2382-2428` (M-29); `SONDE-015.md:1068` (M-25), `:1077` (M-29), `:3400` (E-8);
`Entwurf:2986-2987`, `:2995-2999`.

**Nachrechnungsprüfung:** Abbildung korrekt, Zahlen bestätigt und eingegrenzt.
Eigene Messung (Ton auf Bandmitte, 48 kHz, Crossfade 256):
Bell 50 Hz Q 8 +12 dB **−6,57 dB bei 40 ms**, innerhalb 1 dB **ab 220 ms**;
Bell 50 Hz Q 24 +12 dB −9,57 dB / 600 ms; Bell 100 Hz Q 2 +6 dB −1,67 dB / 20 ms;
Bell 1 kHz Q 2 +6 dB **−0,21 dB / 3 ms**; LowCut 30 Hz Q 0,707 −0,00 dB.
Der Befund gilt also ausschließlich für **tieffrequente, hochgütige** Bänder; bei
gewöhnlichen Einstellungen ist er unhörbar. Die dynamische Teilbehauptung
(Auslenkung startet bei 0 und baut sich erst über die Attack wieder auf) ist am Code
belegt: `zustaendeNullen` nullt `huelle` und `auslenkungDb` mit.

**Erreichbarkeit:** `v1.band.<n>.enabled` ist Hostparameter und nicht topologisch, wirkt
also per FL-Automation — aber erst an einem belegten Slot (NAK-306). Zusätzlich jede
Transaktion mit Typ-/Modus-/Dynamik-/Quellenwechsel, `bandBelegen`, `remove`,
`presetLaden`.

**Kein Duplikat:** T3-14-02 ist die Gegenrichtung (Historie wird zu Unrecht übernommen);
beide Befunde zeigen auf dieselbe fehlende Slot-Lebenszykluskennung, sind aber nicht
dieselbe Zeile.

**Nebenbefund für den Dirigenten (nicht gezählt):** Der Vertrag markiert `enabled` als
`topologisch: false`, der DSP behandelt es über `rampenKompatibel` faktisch topologisch.
Das ist die Stelle, an der die Hostautomation den teuren Wechsel auslöst.

---

## K5 — Auto-Gain ungeklemmt jenseits ±120 dB

**URTEIL: BESTÄTIGT** · Kategorie **LÜCKE** · Schwere **low** · Klasse K3

**Stärkstes gefundenes Gegenargument (hält für die Schwere, nicht für den Befund):** Der
Sender ist nicht angeschlossen — `baueBericht` hat außerhalb der Tests **keinen Aufrufer**
(eigene Suche: nur `SondeProcessor.cpp:1281-1285` als Getter, `TransactionTestMain.cpp` und
`SchemaTestMain.cpp` als Leser). Ein Vertragsbruch auf dem Draht ist heute unmöglich.
Zweitens sind die Konfigurationen absurd. Drittens verletzt die Ableitung selbst keine
Zusage: R4 (`SONDE-015.md:1603-1616`) und M-35 bis M-40 nennen keine Grenze.

**Warum es trotzdem steht:** Die Grenze des Vertrags ist mit einer **nachweislich falschen
Herleitung** begründet. `eq-ipc-v3.schema.json:2324-2329` trägt den `$comment`: „Die Grenze
folgt aus dem Erreichbaren: acht Baender zu je 12 dB im selben Kanalmodus ergeben
hoechstens 96 dB Plateau; 120 ist die Reserve darueber." Das Erreichbare ist größer: RBJ-
Shelves überschwingen bei Q > 0,707, und Cuts dämpfen unbegrenzt. `baueBericht` übernimmt
den Wert ungeklemmt (`NakamaTransaktion.cpp:785-796`), die Ableitung klemmt nicht
(`DspProgramm.cpp:96-135`, nur `mittel > 0` und endlich).

**Geprüfte Stellen (selbst gelesen):** `DspProgramm.cpp:85-136` (ganze Ableitung, 121
Gitterstellen, Diagonalterm), `:266-267`; `DspKern.cpp:210` (Bericht), `:424` (Rampenziel),
`:709-732` (Anwendung); `NakamaTransaktion.cpp:773-816` mit `:785-796`;
`NakamaTransaktion.h:377-391`; `eq-copilot/schemas/v3/eq-ipc-v3.schema.json:2312-2340`;
`nakama-parameter-v2.json` (q 0,15-24 für **alle** Typen, gain ±12 dB);
`DspGoldenTestMain.cpp:2433-2621` (Abschnitt H: flach, 0-dB-Bell, +6-dB-Shelf, Mid/Side,
Dynamik, Output-Trim — keine Grenzprüfung); `SONDE-015.md:2951` (E2-7), `:1083-1088`.

**Nachrechnungsprüfung:** Abbildung korrekt, Zahlen bestätigt.
8× LowShelf +12 dB Q 8 @1 kHz → **−199,77 dB**; Q 24 → −275,52 dB;
8× HighCut 20 Hz Q 0,15 → **+150,46 dB**; 8× HighCut 20 Hz Q 0,707 → +42,99 dB;
2× HighCut 40 Hz Q 0,707 → +11,45 dB; 8× LowShelf +12 dB Q 0,707 → −92,29 dB (innerhalb).

**Eigener Zusatz gegen die Fixierung auf die Zahl 120:** Eine Klemmung auf ±120 dB
beseitigt die Audiofolge nicht — 120 dB sind linear 1e6. Auch die vertragskonformen
+43 dB eines achtfachen 20-Hz-Tiefpasses heben Infraschall und Gleichanteil hörbar an,
weil das Gitter erst bei 20 Hz beginnt. Der Befund sollte deshalb als „fehlende Regel für
extreme Kurven **und** falsch hergeleitete Vertragsgrenze" geführt werden, nicht als reines
Klemmproblem.

---

## K6 — „nahe Nyquist wird Decramping geprüft" ohne Nachweis

**URTEIL: HERABGESTUFT** · Kategorie **HÄRTUNG** (vorgeschlagen war LÜCKE) · Schwere **low**
· Klasse K3 (Plan- und Vertragsabdeckung)

**Stärkstes gefundenes Gegenargument (hält):** Es fehlt keine Regel — die Regel existiert
und ist gemessen. R15 (`SONDE-015.md:2094-2097`) legt fest, dass die Filtergoldens „gegen
die **analytische RBJ-Antwort**" laufen; M-10/M-11 (`:1048-1049`) binden ±0,05 dB typisch
und ±0,1 dB an Extrempunkten an genau diese Referenz; M-13 (`:1051`) verlangt eine im Test
eigenständig ausgeschriebene RBJ-Formel. Ein eingebautes Decramping würde diese Zusagen
**brechen**. Der Entwurfssatz sagt außerdem „wird **geprüft**", nicht „wird gebaut" — er
fordert kein Produktverhalten. Damit fehlt weder eine erreichbare Regel noch ist eine
Zusage verletzt: übrig bleibt ein **unabgeschlossener Prüfauftrag ohne Aktenzeichen**, also
Begründungs-/Nachweisarbeit = HÄRTUNG.

**Was bleibt:** Der Satz steht in einem bindenden Dokument (`Entwurf:2980-2981`) und ist in
`SONDE-015.md:210-211` ausdrücklich unter „die **bindenden** Sätze" zitiert. Meine Suche
über `docs/`, `wissen/`, `design/`, `eq-copilot/` findet zu Decramping/Orfanidis/cramping/
matched-analog **nur** diese beiden Stellen und das Archiv (`docs/archiv/…-0.4-…:2704`) —
keine M-Zeile, keinen E-/R-Entscheid, keine Registerzeile, keinen Planschritt. `CLAUDE.md`
verlangt für so etwas eine Zeile in `docs/offene-punkte.md`.

**Geprüfte Stellen (selbst gelesen):** `docs/FL-Nakama-Sonden-Design-Entwurf.md:2965-3020`,
`SONDE-015.md:198-235` (§1.3), `:1048-1051`, `:2085-2105` (R15 samt Feinheit 2 und 3),
`:3379` (`kNyquistAnteil`); `DspFilter.h:104-131` (Bell), `:109-114` (`kappeNyquist`),
`DspSvf.h:26-30`; `DspGoldenTestMain.cpp:1121-1177` (sieben Prüflinge, höchste
Eckfrequenz 8 kHz); Volltextsuche über das Repo.

**Nachrechnungsprüfung:** Abbildung korrekt, Zahl bestätigt. Bell 16 kHz Q 2 +12 dB,
gemessen bei 19,2 kHz: 44,1 kHz **−6,48 dB**, 48 kHz −5,52 dB, 96 kHz −1,19 dB gegen den
analogen Prototyp. Ergänzend habe ich geprüft, dass die Abweichung bei 16 kHz exakt 0,00 dB
ist (Prewarping auf f0) — die Kompression sitzt ausschließlich oberhalb und unterhalb der
Mitte im obersten Oktavbereich, und derselbe gespeicherte Zustand klingt bei 44,1 gegen
96 kHz dort um bis zu 5 dB verschieden. Das ist RBJ-typisch und mit M-10 vereinbar.

**Folge für den Dirigenten:** eine Registerzeile („Decramping geprüft und verworfen/vertagt,
RBJ bleibt Referenz — R15, M-10") genügt; Code- oder Testarbeit fällt nicht an, solange die
Entscheidung RBJ heißt.

---

## K7 — falsche Begründung der Steuerrate

**URTEIL: HERABGESTUFT** · Kategorie **HÄRTUNG** (vorgeschlagen war „HÄRTUNG oder LÜCKE")
· Schwere **low** · Klasse K5

**Stärkstes gefundenes Gegenargument (hält teilweise):** Keine Zahlenzusage zur
Reaktionszeit existiert, die Hüllkurve selbst folgt der Attack mit voller Audiorate
(M-26 ist erfüllt), E-25 rechnet die Steuerrate ausdrücklich in die 1-ms-Toleranz ein
(`SONDE-015.md:3419`: „die Steuerrate (höchstens 0,18 ms)"), und bei 96/192 kHz ist die
Aussage sogar richtig. Ein Verzug von 0,17 bis 0,34 ms ist praktisch kaum hörbar. Damit
bleibt kein Produktfehler — nur eine falsche Begründung.

**Was bleibt (nicht widerlegbar):** Die Aussage in E-6 (`SONDE-015.md:3397`) und im
Kommentar `DspProgramm.h:64-70` — „8 Samples sind 0,167 ms bei 48 kHz und damit **feiner
als die kürzeste Attack (0,1 ms)**" — ist schlicht falsch: 0,167 ms ist **gröber** als
0,1 ms, bei 44,1 kHz sind es 0,181 ms. Die Vertragsuntergrenze von `attack_ms` ist
tatsächlich 0,1 ms (`nakama-parameter-v2.json`, band_vorlage: min 0,1, max 500). Eine
Begründung, die in einem Entscheid steht und zweimal falsch rechnet, ist ein Riss in der
Beweiskette und gehört berichtigt.

**Geprüfte Stellen (selbst gelesen):** `DspProgramm.h:64-70` (`kDynamikSchritt` samt
Begründung); `DspKern.cpp:557-581` (Mechanik: am Steuerschritt `svfVon = svfNach`,
`svfNach = neuerEntwurf`, `tSchritt = 1 − schrittRest/8`, also **t = 0 am Entwurfssample**);
`DspBankPool.h:65-70`; `nakama-parameter-v2.json` (attack 0,1-500 ms);
`DspGoldenTestMain.cpp:1894-1970` (Sprungantwort A 20 ms, Toleranz 1 ms),
`:1972-2059` (W-2 misst am Steuerschritt, nicht die angewandte Verstärkung dazwischen);
`SONDE-015.md:3377` (Konstantentabelle), `:3397` (E-6), `:3419` (E-25), `:1069` (M-26).

**Nachrechnungsprüfung:** Die Mechanik habe ich am Code durchgezählt und bestätige die
Herleitung: der am Steuerschritt entworfene Satz wirkt dort mit Gewicht 0 und erst
8 Samples später voll; zusammen mit der Phase des Steuertakts ergibt das **8 bis 15 Samples**
= 0,167 bis 0,312 ms bei 48 kHz (0,181 bis 0,340 ms bei 44,1 kHz).
Eigener Zusatz zur Beweislücke: Bei `attack_ms` = 0,1 ms wäre die tatsächliche Gainflanke
rund dreimal langsamer als eingestellt — die Prüftoleranz von 1 ms (E-25) kann das
prinzipiell nicht sehen. Das ist der eigentliche K5-Anteil des Kandidaten.

---

## Tabelle

| Kandidat | Urteil | Kategorie | Schwere | Kernbegründung |
| --- | --- | --- | --- | --- |
| K1 Pfadrampen überdauern `eq_enabled` aus/ein | BESTÄTIGT | DEFEKT | medium | Nachgerechnet +13,41 dB bei Sample 128, blockunabhängig; M-07 trägt die Zusage nicht, aber M-03 „klickfrei" mit der projekteigenen Sprungwache (E-31) und das Grundgesetz „nichts Ungefragtes" tragen sie; einziger Fall des Pakets, der heute per FL-Automation ohne belegtes Band erreichbar ist; `reset()` konserviert den Altstand statt ihn zu heilen |
| K2 Detektorpegel über dem RMS | HERABGESTUFT | LÜCKE | low | DEFEKT widerlegt: der Bau ist wörtlich R7 Feinheit 3 plus M-26, und M-18 definiert `e_db` selbstbezüglich als Detektorpegel nach A/H/R; die M-19-Teilbehauptung ist falsch (Kennlinie liefert bei `e_db ≤ t` bitgenau 0,0). Übrig bleibt die fehlende Pegeldefinition für den Threshold (+2,12 bis +3,01 dB tonal, bis +8,66 dB Rauschen) und der Beweisschatten aller Quadraturton-Tests |
| K3 Rampe mit großem Frequenzsprung | BESTÄTIGT | LÜCKE | medium | Eigene Gegenprobe verschärft: Produktweg +17,4 dB (Shelf) und +22,2 dB (LowCut-Sweep) gegen +1,3 / +1,1 dB bei parameterstetigem Neuentwurf und ≈ 0 dB beim vorhandenen topologischen Crossfade — der Überschuss geht auf das Verfahren, nicht auf die Physik; Stufensweeps sind harmlos (+0,02 dB), gefährlich ist der einzelne Sprung; keine Zusage verletzt, der definierte Ramp-Test misst gegen dieselbe Mischung |
| K4 Topologiewechsel startet alle Bänder kalt | BESTÄTIGT | LÜCKE | medium | −6,57 dB bei 40 ms, 1 dB erst ab 220 ms (Bell 50 Hz Q 8 +12 dB) bestätigt; Kaltstart ist über E-8 entschieden, dessen beide Begründungen aber nachweislich nicht tragen (5,3 ms Fade gegen 20-600 ms Einschwingen; §44.2 verbietet Teilen zwischen **Pfaden**, nicht zwischen Bänken desselben Pfades); unhörbar bei gewöhnlichen Bändern (1 kHz Q 2: −0,21 dB) |
| K5 Auto-Gain ungeklemmt | BESTÄTIGT | LÜCKE | low | −199,8 dB und +150,5 dB nachgerechnet; der `$comment` der Schemagrenze leitet das Erreichbare falsch her (Shelf-Überschwinger, Cuts); Sender von `state_report.dsp` hat außerhalb der Tests keinen Aufrufer, deshalb heute kein Draht-Vertragsbruch; eine Klemmung auf ±120 dB löst die Audiofolge ohnehin nicht |
| K6 Decramping ohne Nachweis | HERABGESTUFT | HÄRTUNG | low | Keine fehlende Regel: R15/M-10/M-13 legen RBJ als gemessene Referenz fest, Decramping würde sie brechen; „geprüft" fordert keinen Bau. Bestätigt bleiben −6,48 dB gegen den analogen Prototyp bei 19,2 kHz / 44,1 kHz und die Tatsache, dass zum Prüfauftrag nirgends ein Entscheid oder eine Registerzeile existiert |
| K7 Begründung der Steuerrate | HERABGESTUFT | HÄRTUNG | low | Arithmetik bestätigt (0,167 bzw. 0,181 ms gegen 0,1 ms Vertragsminimum, volle Gainwirkung nach 8-15 Samples); kein Produktfehler, keine Zahlenzusage, E-25 rechnet die Steuerrate in die Toleranz ein — falsch ist allein die Behauptung in E-6 und `DspProgramm.h:64-70`; Nebenpunkt: die 1-ms-Prüftoleranz kann den Verzug prinzipiell nicht sehen |

---

## Gelesener Scope dieser Gegenprüfung

**Vollständig gelesen:** `AGENTS.md`; `CLAUDE.md` (im Kontext);
`eq-copilot/plugin/dsp/DspKern.cpp:1-1104`; `DspKern.h:1-451`; `DspFilter.h:1-369`;
`DspSvf.h:1-166`; `DspProgramm.cpp:1-272`; `DspProgramm.h:1-222`.

**Gezielt gelesen:** `DspBankPool.h:58-107`, `DspBankPool.cpp:1-80`;
`eq-copilot/plugin/sonde/SondeProcessor.cpp:143-165,228-437,500-570,1270-1446`,
`SondeProcessor.h:429, 533-551`;
`eq-copilot/plugin/state/NakamaTransaktion.cpp:320-348,700-821`,
`NakamaTransaktion.h:328-391`;
`eq-copilot/plugin/tests/DspGoldenTestMain.cpp:425-447, 1040-1177, 1400-1553, 1660-1840,
1880-1970`;
`eq-copilot/schemas/state/nakama-parameter-v2.json` (Kopf, Regeln, `global` und
`band_vorlage` vollständig als Werte ausgelesen);
`eq-copilot/schemas/v3/eq-ipc-v3.schema.json:2312-2340`;
`docs/beweise/SONDE-015.md:196-235, 1020-1119, 1540-1780, 2085-2105, 2930-2951, 3370-3429,
3436-3460`;
`docs/FL-Nakama-Sonden-Design-Entwurf.md:2955-3024,3085-3110`;
`docs/audits/2026-09-15-tiefenaudit/BEFUNDE.md:1-125, 343-351, 412, 495-503, 545, 588-591,
601`;
`docs/offene-punkte.md:100,121-128,264,314`;
Scratchpad des Finders: `p15-paket2-dsp-numerik.md` (vollständig), `p15-zentral-gegenrechnung.py`.

**Repo-weite Suchen:** `Decramp\|Orfanidis\|cramping\|matched-analog` (nur Entwurf, SONDE-015
§1.3, Archiv); `autoGainDb\|auto_gain_db` (kein Produktsender); `dspBericht\|DspBericht`
(nur Getter und Tests); `zustaendeNullen`; `dsp/Dsp` in `BEFUNDE.md` (T3-01-01, T3-01-04,
T3-05-02, T3-14-02 — keine Überschneidung); `rampe\|kalt\|Detektor\|Nyquist\|Auto-Gain` in
`docs/offene-punkte.md`.

**Nicht gelesen:** `NakamaState.cpp`, `NakamaPreset.*`, Broker, JUCE-Wrapper, die übrigen
Testkörper, `HoerMarkierung.h` (die Hörmarkierungs-Verdachte des Finders gehören nicht zu
den sieben Kandidaten).

**Eigene Skripte (Scratchpad, Nachrechnung abgeschriebener Formeln):**
`p15-gegen2-nachrechnung.py` (K1 blockweise, K2 Detektor gegen RMS, K3 Rampenläufe samt
Zwischenentwürfen, K4 Kaltstart, K5 Auto-Gain-Gitter, K6 Cramping, K7 Arithmetik),
`p15-gegen2-alternativen.py` (K3: Rampe gegen Sofortsprung, lange Rampe, topologischen
Crossfade und parameterstetigen Neuentwurf) sowie eine Einzeilenprüfung zum
Stufensweep 2 kHz → 20 Hz.
