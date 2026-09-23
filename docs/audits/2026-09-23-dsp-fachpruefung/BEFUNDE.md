# DSP-Fachprüfung am Maßstab von J. O. Smith, „Introduction to Digital Filters" (23.09.2026)

**Auftrag (User, 23.09.2026, wörtlich):** „https://ccrma.stanford.edu/%7Ejos/fp/
ich hab diese seite gefunden, sie wirkt auf mich wie ein goldschatz an
fachinformationen zu EQ coding. ich möchte dass du unseren nakama plan
ausführlich durchgehst, die infos dieser Seite aufsaugst und mir dann erstmal
mitteilst ob das nützlich für uns sein könnte. ich bin kein audio engeneer
oder coder, ich kann daher nicht beeurteilen wie fachlich hochwertig die
mathematik oder code ist den wir eingebaut haben bis jetzt, oder wie fachlich
korrekt der plan geschrieben ist. du hast die ausdrückliche erlaubnis soviele
agenten wie benötigt werden zu nutzen. diese aufgabe erfordert sorgfalt"

**Form:** Nebensession, am Produkt nur lesend. 13 Agenten (Opus 5.5): vier
Buchleser (alle 390 Seiten), je ein Prüfer für EQ-Code, Messcode und Plan, eine
Quellenrecherche, eine Nachrechnung; danach vier adversariale Gegenprüfer, die
jede tragende Zahl unabhängig neu rechneten. DSP-02 hat die Session zusätzlich
selbst nachsimuliert. Gerechnet wurde an Python-Nachbauten der Produktformeln,
nicht am Binary ([`skripte/`](skripte/README.md)).

**Stand:** HEAD `4a831da3`, DSP-Kern zuletzt geändert in `3d4a5a8e`.

**Buchquelle:** CCRMA sperrt KI-Agenten per robots.txt. Gelesen wurde die
textgleiche Ausgabe bei DSPRelated
(<https://www.dsprelated.com/freebooks/filters/>, Stand der Druckausgabe
W3K 2007/08, ohne Fußnoten). Vor dem Sperrhinweis wurden einzelne CCRMA-Seiten
abgerufen (Index, Peaking-EQ, LTV-Anhang, SASP-Startseite), danach keine mehr.

## 1. Urteil

**Das Buch.** Nützlich als verlässliches Lehr- und Prüfbuch für die Grundlagen,
nicht als Bauanleitung für Nakamas offene Probleme. Es ist eine Einführung
(„gentle introduction", Kurs für Musik-Doktoranden seit 1984), fachlich
verlässlich und in der Fachliteratur zitiert. Für Nakama leistet es dreierlei:

1. Es **bestätigt** den EQ-Kern: JOS' Peaking-EQ ist nach Umrechnung
   (g = A², Q_JOS = A·Q_RBJ) exakt die RBJ-Glocke (Koeffizientendifferenz
   ≤ 4,4e-16), dazu Frequenzgang, Stabilität, Polradius, Serienschaltung,
   Minimalphase.
2. Es **erklärt die Ursachen** der offenen Punkte: Frequenzverzerrung der
   bilinearen Transformation (DSP-09), Zustandssprung beim Koeffizientenwechsel
   (Zustandsraum und Ähnlichkeitstransformation, DSP-01/DSP-02),
   Allpass-Phase einer Weiche (DSP-04), LTI-Voraussetzung jedes Frequenzgangs
   (Auto-Gain dynamischer Bänder bleibt Näherung), Ausklingzeiten (DSP-13).
3. Es liefert **Prüfregeln** für Goldens (§5).

Keine Lösungen enthält es für Decramping, zeitvariante Filter und Glättung,
TPT-SVF, Detektoren dynamischer EQs, Lautheitsnormen und Spektralstatistik;
dafür gelten die Quellen in §6, für die Messseite JOS' Schwesterbuch
„Spectral Audio Signal Processing" (SASP). In der gelesenen Fassung stehen
einzelne Druck- und Listingfehler (z. B. Vorzeichen in einer
Summenformel, `boost()` rechnet Q = fs/bw statt fc/bw); ob die gepflegte
CCRMA-Fassung sie korrigiert, ließ sich nicht prüfen.

**Der Code.** Die Filtermathematik ist korrekt und auf professionellem Niveau:
RBJ-Formeln wortgetreu, Boost und Cut exakt reziprok, der Simper-SVF ist
algebraisch identisch mit RBJ (mpmath: 1e-53 dB; in double ≤ 4e-7 dB),
durchgehend double (float32 verstimmte eine Q-24-Resonanz bei 20 Hz um bis
13 dB), bitgenauer Nullpfad, sauberer NaN-, Denormal- und Überlaufriegel, null
Latenz. Der Crossfade überschreitet nie die höchste stationäre Spitze; der SVF
ist unter Modulation beweisbar stabil. Im Messkern sind PSD-Normierung, Energie
vor dB, Gating mit festem Speicher und True Peak (besser als das
ITU-Beispielfilter, in der EBU-Toleranz) sauber. **Einen Rechenfehler gegen die
eigene Spezifikation fand keiner der Prüfer.** Angreifbar sind
Entwurfsentscheidungen der Spezifikation (Rampen, Mono-Bass, Detektor,
Güte-Semantik, Mix), einzelne Messverfahren (Einheit, Statistik, Auflösung,
Vergleichspegel) und die Testsemantik.

**Der Plan.** Überwiegend fachlich korrekt: Normfakten (BS.1770-5,
EBU Tech 3341 V4), minimalphasig ohne Latenz, Welch mit Hann, Übertragungs- und
Laufzeitschätzer. Einige Begründungen tragen nicht („stabil, weil das
Stabilitätsdreieck konvex ist"; „Mix 0,5 = halbe Kurve"; „ohne ein starkes Delta
zu übersteuern"; Tail-Begründung), einige Stellen widersprechen sich (§4).

## 2. Befunde am EQ-Kern

Einordnung wie im Register: Defekt · Lücke · Härtung · Produktfrage · Doku.
„Nachbau" heißt Python an abgeschriebenen Formeln.

| ID | Befund | Einordnung | Wirkung für den Musiker | Belege | Bekannt · Ort |
|---|---|---|---|---|---|
| DSP-01 | **Koeffizientenrampe statischer Bänder überschwingt im Tiefbass und bei steilen Shelves.** Innerhalb der Sprungkriterien (Frequenz ≤ ×2, Güte ≤ ×4, Gain ≤ 20 dB) mischt der Kern die fünf DF2T-Koeffizienten 256 Samples linear, der Zustand wandert mit. Nachbau über 1 116 Grenzfälle mit dem Maß R-311-13: Verfahrensanteil über der Sweep-Physik (Referenz TPT-SVF) > 1 dB in 31 %, > 3 dB in 9,7 %, > 6 dB in 2,8 %; Maximum High-Shelf 80→40 Hz, Q 24→6, +10→−10 dB: +22,8 dB, davon bis +17 dB Verfahren. „Stabil, weil konvex" belegt nur eingefrorene Zwischenentwürfe (Laroche 2007); instabil wird nichts, es überschwingt. Die +17,41/+22,20 dB aus Tiefenaudit Phase 15 stammen von vor NAK-311 und laufen heute als Crossfade. | Lücke (mittel) | Preset laden, Undo/Redo, Wert tippen oder eine Automationsstufe knapp unter der Sprunggrenze: im Tiefbass oder bei steilen Shelves für Millisekunden ein Wummern bis etwa 10–20 dB über vorher und nachher. | `eq-copilot/plugin/dsp/DspKern.cpp:39-52`, `:360-366`, `:586-603`; `eq-copilot/plugin/dsp/DspProgramm.h:150-212`; `docs/beweise/SONDE-015.md:3412`; [v5_rampe.py](skripte/gegenpruefung-eq-1/v5_rampe.py) | teilweise NAK-336 (d); Ort S25p (NAK-340) |
| DSP-02 | **Dynamische Bänder (SVF-Bell) überschwingen bei schneller Güte-Absenkung.** Der SVF-Bell gibt x + k(A²−1)·v1 aus, der Bandzustand v1 wächst mit 1/k. Sinkt Q innerhalb einer Rampe (bis ×4), steigt k schneller, als v1 abklingt. Nachbau des Produktverfahrens (Entwurf alle 8 Samples, lineare Zwischenwerte, 256-Sample-Rampe), +12 dB, Q 24→6: +9,7 dB bei 50 Hz, +7,9 dB bei 1 kHz; DF2T-Rampe und Crossfade 0 dB; Q-Anhebung 0 dB. | Lücke | Wer an einem dynamischen Band die Güte schnell absenkt (Regler, Automation, Preset), hört kurz eine Anhebung bis etwa 10 dB über dem eingestellten Pegel. | `eq-copilot/plugin/dsp/DspSvf.h:102-112`; `eq-copilot/plugin/dsp/DspKern.cpp:871-895`; [r5e_qsprung.py](skripte/nachrechnung/r5e_qsprung.py); [svf_qrampe.py](skripte/eigen/svf_qrampe.py) | neu; Ort S25p oder eigene Nacharbeit |
| DSP-03 | **Delta-Hören mit festem +12-dB-Ausgleich ohne Schutz.** Gebaut, nicht verdrahtet: nur Tests setzen die Hörmatrix auf Delta. Delta = (Processed − Dry)·3,98 (+12 dB), danach nur ein Endlichkeitsriegel. Bell +12 dB bei f0: +21,5 dB über Dry; Low-Shelf +12 dB auf rosa Rauschen (Spitzen −6 dBFS): Delta-Spitzen +9 dBFS; Output-Trim +24 dB: +35 dB. Der Entwurf definiert Delta als Preview minus pegelangepassten Zustand und deckelt den Ausgleich auf ±6 dB mit gemeinsamem Absenken; die Begründung „ohne ein starkes Delta zu übersteuern" (SONDE-015 R10) ist rechnerisch widerlegt. | Defekt, ruhend | Heute nicht erreichbar. Sobald Delta bedienbar wird: nach kräftigem Bass-Boost beim Umschalten ein Sprung bis etwa 20 dB und Übersteuerung – Gehörschutzfrage. | `eq-copilot/plugin/dsp/DspKern.cpp:1436-1448`; `eq-copilot/plugin/dsp/DspProgramm.h:217`; `docs/beweise/SONDE-015.md:1506`, `:1773`; `docs/FL-Nakama-Sonden-Design-Entwurf.md:2701`, `:2730-2736`; [v1_delta.py](skripte/gegenpruefung-eq-1/v1_delta.py) | neu (NAK-370 betrifft nur A2); vor der Verdrahtung der Hörmatrix (S29–31) beheben |
| DSP-04 | **Mono-Bass ohne Phasengleichheit.** Nur S läuft durch einen Butterworth-Hochpass 2. Ordnung, M bleibt unberührt, keine Kompensation. Unabhängig von fc: Rest-Seite −12,3 dB bei fc/2 mit 137° Drehung; ein hart links gepannter Ton bei fc/2 erscheint mit R/L +3,0 dB; Übersprechen −8,0 dB bei 2·fc, −14,8 dB bei 4·fc, −23 dB bei 10·fc. LR4 auf S plus Allpass auf M ergäbe −30/−54/−86 dB, verzögert aber die Mitte im Bass (3,75 ms bei 120 Hz) und kollidiert mit dem Entwurf („keine heimliche Allpass-/Delay-Korrektur"). | Lücke + Produktfrage | Mono-Bass bei 120 Hz rückt einen hart links liegenden 70-Hz-Ton leicht nach rechts statt in die Mitte; links gepannte tiefe Mitten sprechen bis etwa 500 Hz ins rechte Signal über. | `eq-copilot/plugin/dsp/DspKern.cpp:998-1009`; `eq-copilot/plugin/dsp/DspProgramm.cpp:278-279`; `eq-copilot/schemas/state/nakama-parameter-v2.json:86`; `docs/FL-Nakama-Sonden-Design-Entwurf.md:2681-2682`; [v1_monobass.py](skripte/gegenpruefung-eq-2/v1_monobass.py) | neu; Karte vor Abnahme S26–28 |
| DSP-05 | **Attack-Boden durch das feste 10-ms-Pegelfenster.** Im Standardbegriff „Durchschnitt" liegt ein 10-ms-Leistungsmittel vor der Attack-Stufe. Bell 1 kHz Q 1, Zeit bis −1 dB unter Endwert: Attack 0,1 ms → 16,4 ms (ohne Fenster 0,8 ms), 1 ms → 17,3 ms, 10 ms → 30 ms. Weit über der Schwelle greift es früher (18 dB darüber: 2,8 ms). U45 überließ das Fenster der Technik; „einstellbar wie heute" war Fragetext, kein User-Wort. | Produktinformation + Lücke bis NAK-331 | Ein dynamisches Band mit 0,1 ms Attack verhält sich knapp über der Schwelle wie eines mit etwa 15 ms; schnelles Zähmen von Transienten gibt es erst mit dem noch nicht bedienbaren Modus „Spitze". | `eq-copilot/plugin/dsp/DspFilter.h:289`, `:363-368`; `eq-copilot/plugin/dsp/DspKern.cpp:866-867`; `design/abnahmen/2026-09-19-dynamik-schwelle-je-band-umschaltbar-u45.md`; [v4_attack.py](skripte/gegenpruefung-eq-1/v4_attack.py) | Mechanismus bekannt (M-127), Boden neu; Ort NAK-331 |
| DSP-06 | **Detektor an die Band-Güte gekoppelt.** Der Detektor-Bandpass nutzt Frequenz und Q des Bandes (Spez R7). Schmale Bänder reagieren träge: Bell 100 Hz Q 24 erreicht −1 dB erst nach 180–200 ms, gleich bei Attack 0,1 oder 10 ms. Dieselbe Schwelle bedeutet je Güte etwas anderes: Rauschen liest bei Q 24 um 14 dB tiefer als bei Q 0,707. Dynamische Shelves hören an der Ecke: Low-Shelf 200 Hz Q 0,707 ist bei fc/4, wo er wirkt, 9 dB unempfindlich und bei 4·fc, wo er nicht wirkt, genauso. Marktübliche Detektoren sind bandbezogen, bieten aber eine Entkopplung (Pro-Q „Free"). | Spezlücke + Produktfrage | Ein schmales dynamisches Bassband greift erst nach etwa 0,2 s voll, egal wie kurz die Attack steht; ein dynamischer Bass-Shelf reagiert auf die Gegend um 200 Hz statt auf den Tiefbass, den er absenkt. | `eq-copilot/plugin/dsp/DspProgramm.cpp:347-349`; `docs/beweise/SONDE-015.md:1711`; [v2_detektor.py](skripte/gegenpruefung-eq-2/v2_detektor.py) | neu; Karte vor Abnahme S26–28 |
| DSP-07 | **Güte-Semantik und Flanken.** Ein Q-Bereich (0,15–24) und ein Default (1,0) für alle sechs Typen; Shelves mit RBJ-Q statt Slope. Bei ±12 dB: Shelf-Überschwinger ±0,78 dB (Q 1), ±4,6 dB (Q 2), ±25 dB (Q 24); Low-/High-Cut mit Default Q 1: +1,25 dB Buckel über der Ecke, Q 24: +27,6 dB Resonanz, Q 0,15: −16,5 dB an der Ecke; Cuts nur 12 dB/Okt (Pro-Q: bis 96 dB/Okt). | Produktfrage | Ein auf Low-Cut umgestelltes Band hebt mit der Grundgüte knapp über der Eckfrequenz 1,25 dB an; mit hoher Güte wird aus Cut oder Shelf ein Resonanzfilter; steilere Cuts nur durch Stapeln von Bändern. | `eq-copilot/schemas/state/nakama-parameter-v2.json:166-177`; `eq-copilot/plugin/dsp/DspFilter.h:133-218`; [v3_guete.py](skripte/gegenpruefung-eq-2/v3_guete.py) | teilweise O-04/F10 (nur Shelves, ohne Registerzeile); Karte vor Abnahme S26–28 |
| DSP-08 | **Mix mit phasendrehenden Filtern.** Mix mischt Dry und Wet linear. Low-Cut 100 Hz Q 4 bei Mix 0,5: −15,8 dB Einbruch bei 69 Hz (Mix 0,75: −27,8 dB); auch Q 0,707: −7,6 dB statt −6 dB. Glocken praktisch unberührt. SONDE-015 begründet „Mix 0,5 = halbe Kurve, Dry und Wet hochkorreliert" – an Cut-Ecken falsch (130–170° Phasenabstand). Nebenbefund: Der Vertragstext zu `mix` („VOR Auto-Gain und Output-Trim") widerspricht Kette und Code (Auto-Gain vor Mix). | Planfehler in der Begründung + Produktfrage | Einen steilen Low-Cut halb beimischen erzeugt knapp unter der Eckfrequenz ein Loch von −16 dB, das weder das trockene noch das bearbeitete Signal hat. | `eq-copilot/plugin/dsp/DspKern.cpp:1027-1049`; `docs/beweise/SONDE-015.md:1590-1594`; `eq-copilot/schemas/state/nakama-parameter-v2.json:109`; [v4_mix.py](skripte/gegenpruefung-eq-2/v4_mix.py) | neu; Karte vor Abnahme S26–28 |
| DSP-09 | **Höhen-Verformung (Cramping): Zahlen für die spätere Karte.** RBJ trifft Frequenz und Pegel der Spitze exakt, staucht den Gang aber zur Nyquistgrenze. +12-dB-Glocke bei 44,1/48 kHz: mehr als 1 dB Abweichung vom Analogvorbild ab f0 ≈ 3,5–7,7 kHz (je nach Q), an der Kappungsgrenze bis 8,8 dB bei 5- bis 9-fach schmalerer Glocke; hochgütige Shelves bis etwa 28 dB; bei 88,2/96 kHz Glocken höchstens 1,8 dB. Vicaneks matched-Entwurf senkt das auf ≤ 1,4 dB (Boost) bzw. ≤ 1,9 dB (Cut) bei 44,1 kHz und ≤ 0,13 dB ab 88,2 kHz; Orfanidis versagt bei breiten Glocken nahe Nyquist (Bedingung f0 < (1 − 1/(2Q))·fs/2); für Shelves fand sich keine belegte Primärquelle. Wer nur die statischen Bänder decrampt, zerstört die heutige Gleichheit statisch = dynamisch (der SVF ist bilinear) um bis etwa 9 dB im obersten Oktavbereich – beide Wege gehören zusammen. | Information zu NAK-332 (U46: vertagt, bleibt so) | Dasselbe Höhen-Preset klingt bei 44,1 und 96 kHz verschieden; eine Glocke bei 16 kHz wirkt bei 44,1 kHz deutlich schmaler als eingestellt. | [r1_cramping.py](skripte/nachrechnung/r1_cramping.py); [r2_decramping.py](skripte/nachrechnung/r2_decramping.py); `design/abnahmen/2026-09-19-decramping-vertagt-u46.md` | bekannt NAK-332 |
| DSP-10 | **Testsemantik der DSP-Goldens.** Der Test „Zipper < −100 dBFS" vergleicht die Rampe mit einer Referenz desselben Verfahrens (DF2T, lineare Koeffizientenmischung) und begrenzt Stufen auf das Sechsfache des Rampenschritts (etwa −33 dBFS); ein Modulationsartefakt misst er nicht. Die Filtergoldens decken Q 0,15 und 24, 20 Hz, die Nähe von 0,45·fs, Mono-Bass und Phase nicht ab; Bell 20 Hz Q 24 +12 dB bräuchte nach der Buchregel N > 7/(1−R) eine Impulsantwort von etwa 5 s statt 32 768 Samples. Von zwei Prüfern unabhängig gelesen. | Lücke (Beweis) | Die grünen Tests belegen Treue zur eigenen Formel, nicht die Klangqualität an den Extremen. | `eq-copilot/plugin/tests/DspGoldenTestMain.cpp:3953-4050`, `:3623-3674`, `:383-396` | neu; Ort Goldens S26–28 und G6 |
| DSP-11 | **Übergangszeiten in Samples statt Millisekunden.** Topologie-Crossfade, Hörmatrix-Blende, Hostbypass-Blende und Pfadrampen sind fest 256 Samples: 5,8 ms bei 44,1 kHz, 1,3 ms bei 192 kHz. Entwurf §41.2 verlangt für den Hörmatrix-Wechsel 5–20 ms (ab 88,2 kHz unterschritten); die Karten U44/U48 nannten „etwa 5 ms". Die Lesart, der Entwurf verlange 20–100 ms Parameterrampen, ist widerlegt: das ist eine Erlaubnis für die Vorschau. | Härtung + Planfehler | Bei 96 oder 192 kHz blenden A/B, EQ-Schalter und Bypass zwei- bis viermal schneller und können bei Bass eher knacken. | `eq-copilot/plugin/dsp/DspProgramm.h:87-91`; `docs/FL-Nakama-Sonden-Design-Entwurf.md:2731-2734`; `docs/beweise/SONDE-015.md:1570`; [v6_zeiten_kennlinie_ms.py](skripte/gegenpruefung-eq-2/v6_zeiten_kennlinie_ms.py) | neu |
| DSP-12 | **Kennlinie „Kniebreite".** g = r·min(1, max(0, e−t)/12) ist eine lineare Rampe mit harten Ecken bei t und t+12 dB; Range stellt Tiefe und wirksame Ratio zugleich (−3 dB → 1,33:1, −6 dB → 2:1, −12 dB → ∞:1 über 12 dB). | Härtung (Benennung) + Produktfrage | Range stellt Tiefe und Härte zugleich ein; bei −12 dB hält das Band seinen Bereich über 12 dB wie ein Limiter fest. | `eq-copilot/plugin/dsp/DspFilter.h:428-442`; `docs/beweise/SONDE-015.md:1705-1707` | neu |
| DSP-13 | **Tail 0 trotz langer Nachklänge.** Die Sonde meldet Tail 0; Bell 20 Hz Q 24 +12 dB klingt 5,3 s nach (dynamisch wirksam +24 dB: 10,5 s), Low-Cut 20 Hz Q 24: 2,6 s. Die Begründung in SONDE-015 R9 F5 („kein Tail im Sinne des Hostvertrags") widerspricht der Steinberg-Doku zu `getTailSamples`. Dass FL deshalb abschneidet, ist unbelegt; Smart Disable wirkt laut Image-Line nur live, nicht beim Rendern. | Härtung (niedrig) + Doku | Nur bei extremen Tiefbass-Einstellungen mit Smart Disable im Live-Abspielen könnte das letzte, leise Ausklingen abreißen. | `eq-copilot/plugin/sonde/SondeProcessor.h:235`; `docs/beweise/SONDE-015.md:1757`, `:1104`; [v3_tail.py](skripte/gegenpruefung-eq-1/v3_tail.py) | Technikentscheid R9 F5, nicht registriert |
| DSP-14 | **Advisor-Güte.** q = 24/Bänderzahl (1/Bandbreite in Oktaven) statt der RBJ-Beziehung, die `HoerMarkierung.h` selbst nutzt: Vorschläge 1,44-fach breiter als der Befund. Das Schema klemmt 0,1–20, DSP-Vertrag und Entwurf verlangen 0,15–24. Heute unerreichbar: Vorschläge sind `manual`, das Plugin dekodiert sie nicht. | Härtung (ruhend) + Vertragswiderspruch | Ein künftiges „Vorschlag → EQ" schlüge eine etwa anderthalbmal breitere Absenkung vor, als der Befund belegt. | `broker/src/coordinator/proposal.rs:796-812`; `eq-copilot/schemas/v3/eq-ipc-v3.schema.json:1973`; `eq-copilot/plugin/state/NakamaParameter.cpp:191`; `eq-copilot/plugin/src/HoerMarkierung.h:62-66`; [v2_advisor_q.py](skripte/gegenpruefung-eq-1/v2_advisor_q.py) | neu |
| DSP-15 | **Auto-Gain-Gitter.** 121 log-Stellen: Bell +12 dB Q 24 lässt den Ausgleich beim Verschieben innerhalb einer Gitterzelle um 0,40 dB schwanken (Q 12: 0,22 dB; Q ≤ 4: < 0,01 dB). Die Beispielzahlen in SONDE-015 zur Q-12-Glocke sind nicht reproduzierbar; die Richtung der Begründung stimmt. | Härtung (niedrig) | Eine schmale, starke Anhebung mit AUTO durchs Spektrum gefahren lässt den Pegel um bis zu 0,4 dB wackeln. | `eq-copilot/plugin/dsp/DspProgramm.h:241-243`; `eq-copilot/plugin/dsp/DspProgramm.cpp:76-83`; `docs/beweise/SONDE-015.md:1621`; [v5_autogain.py](skripte/gegenpruefung-eq-2/v5_autogain.py) | neu |
| DSP-16 | **M/S-Wortwahl.** Der Entwurf nennt M/S „energienormiert, M = (L+R)/√2", der Code rechnet (L±R)/2. Hörbar folgenlos; die gebaute Wahl hält die Schwelle dynamischer M/S-Bänder konsistent und ist die bessere. | Doku | – | `docs/FL-Nakama-Sonden-Design-Entwurf.md:2657-2658`, `:2985`; `eq-copilot/plugin/dsp/DspKern.cpp:998-999` | neu |

## 3. Befunde am Messkern

| ID | Befund | Einordnung | Wirkung für den Musiker | Belege | Bekannt · Ort |
|---|---|---|---|---|---|
| DSP-17 | **Leistungsdichten werden als Bandleistungen verrechnet.** Die Bänder tragen Dichten (FS²/Hz, Mittel über die Bins), der Vertrag nennt keine Einheit. `bandpassung` („Anteil seiner linearen Leistung"), Screening und Confidence summieren Dichten. Rosa Rauschen, Anteil 8–16 kHz: 0,19 % statt 10,9 %. Rangumkehr: Quelle A mit 40 % ihrer Leistung im Höhenband erhält 0,0067, Quelle B mit 30 % erhält 0,138. Live-Gruppen streuen bei Weißrauschen um 6 dB. Maskierung ist kaum betroffen (Breitenfaktor kürzt sich). | Defekt (Verbraucher) + Lücke (Vertrag) | Der Advisor nennt bei Höhenbefunden eher die basslastige Spur als Verursacher statt der Spur, die in den Höhen Energie hat. | `eq-copilot/plugin/core/analysis/featureengine/Spektrum.h:426-429`, `:488-495`; `broker/src/coordinator/hypothese/zusammenhang.rs:185-212`; `broker/src/coordinator/hypothese/screening.rs:144-168`; `eq-copilot/schemas/v3/flatbuffers/nakama_telemetry_v1.fbs:184-206`; [b3_b6_baender.py](skripte/gegenpruefung-pegel/b3_b6_baender.py) | neu |
| DSP-18 | **Der Vergleichspegel ist kein Lautheitsabgleich.** Gemessen wird das trockene Signal gegen das Signal hinter der Hörmarkierung, nie Vorher gegen Nachher. Ohne aktive Markierung ist der Wert exakt 0 dB und hebt trotzdem die Sperre „ohne Lautheitsabgleich" auf. Ungewichtet, ohne ±6-dB-Deckel, nie auf Audio angewandt. Der Entwurf verlangt LUFS-Abgleich (§15; §41.2: LUFS-I ab 10 s, Median der LUFS-S, darunter vorläufig K-gewichtet, Deckel ±6 dB). Ungewichtet wäre ein Kandidat mit −6-dB-Low-Shelf um 1,56 LU lauter. Das Abnahmeblatt vom 24.08.2026 behauptet in einer Tabellenzeile (Claude-Text) das Gegenteil. | Abweichung von der Spezifikation + Matrixlücke (ruhend) | Nakama verbucht einen Blindvergleich als „lautstärkeangeglichen", ohne Vorher und Nachher je verglichen zu haben; gewinnt die lautere Version, bemerkt es das nicht. | `eq-copilot/plugin/core/analysis/Vergleichspegel.h:205-235`, `:371-412`; `eq-copilot/plugin/src/PluginProcessor.cpp:943-972`; `docs/FL-Nakama-Sonden-Design-Entwurf.md:1117`, `:2719-2736`; `design/abnahmen/2026-08-24-auto-gain-schalter.md:56`; [b9_abgleich.py](skripte/gegenpruefung-pegel/b9_abgleich.py) | neu; vor einem sichtbaren Vorher/Nachher |
| DSP-19 | **Die Experiment-Statistik ist zu optimistisch.** Die Reihe ist „Resultatfenster minus gepooltes Baselinemittel", dessen Unsicherheit fehlt; Perzentil-Block-Bootstrap mit Block 4. Unter der Nullhypothese verfehlt das 95-%-Intervall die 0 in 49/28/22 % (n = 8/20/40; Grenzwert etwa 17 % statt 5 %). Bis 4 Fenster gibt es nur ein Ziehungsmuster, das Ergebnis ist dann immer „Änderung"; der Golden hält das als Sollverhalten fest. Experimente haben keine Mindestfensterzahl. Auf Produktebene bei n = 8 etwa 48 % Fehlalarme. Heute unsichtbar: kein UI-Aufrufer, das Urteil steht wegen der Guardrails auf „vergleich_nicht_gueltig". | Defekt, ruhend | Sobald Vorher/Nachher angezeigt wird, behauptet ein kurz nachgemessener Versuch eine Änderung, die es nicht gab: bis etwa 1,3 s immer, bis etwa 4 s in 35–75 % der Fälle. | `broker/src/coordinator/experiment_verdrahtung.rs:1009-1034`; `broker/src/coordinator/experiment.rs:378-416`, `:1530-1531`, `:1572-1607`; `eq-copilot/plugin/tests/Sonde013ExperimentGoldenTest.cpp:144-146`; [b1_null.py](skripte/gegenpruefung-statistik/b1_null.py) | Gegenseite bekannt (T3-06-03, W29); Ort NAK-316/S25o |
| DSP-20 | **Auflösung fest statt samplerateabhängig.** FFT-Längen 16384/4096 fest: bei 96/192 kHz sind 60/94 von 221 Bändern ohne Bin. Die M1-Resonanzsuche findet bei 48 kHz nichts unter etwa 45 Hz, bei 96 kHz unter etwa 100 Hz, bei 192 kHz unter etwa 500 Hz; M1-LTAS zeichnet interpolierte Lücken wie gemessen, und die Oberfläche sagt nicht, dass dort nicht gesucht wird. | Lücke | Bei 48 kHz findet Nakama keine Subbass- oder Raummoden-Resonanz unter etwa 45 Hz, bei 96 kHz nichts unter etwa 100 Hz, ohne es zu sagen. | `eq-copilot/plugin/core/analysis/FeatureEngine.h:376-383`; `eq-copilot/plugin/src/AnalyseEngine.cpp:596-644`, `:1005-1013`; [b6_m1_voll.py](skripte/gegenpruefung-pegel/b6_m1_voll.py) | neu |
| DSP-21 | **Welch-Streuung als musikalische Schwankung (M1).** Perzentile über Einzelperiodogramme: reines Mono-Rauschen in einem 1-Bin-Band meldet in 39 % „Der Pegel kommt in Wellen"; echte Bewegung wird bei rauschartigem Inhalt um etwa 5 dB übertrieben. Tonale Resonanzen stimmen; die Werkzeugempfehlung kippt nicht. | Lücke (kleine Textwirkung) | Bei rauschartigem Material in schmalen Bändern (etwa 70–800 Hz) kann die Resonanzkarte Wellen melden, obwohl der Pegel konstant ist. | `eq-copilot/plugin/src/AnalyseEngine.cpp:500-503`, `:801-836`; `eq-copilot/plugin/src/Diagnose.cpp:205-214`; [b5_welch.py](skripte/gegenpruefung-statistik/b5_welch.py) | neu |
| DSP-22 | **Ereignisdetektor ohne absolute Schwelle.** Fluss aus log-Bandmitteln gegen den Vorframe, Schwelle Median + 3·mittlere (nicht mediane) Absolutabweichung, kein Maximumfilter (der Entwurf verlangt SuperFlux). Hauptursache der Fehlauslösungen ist die rein relative Schwelle: Weißrauschen 13–24 Ereignisse in 30 s unabhängig vom Pegel, ein sauberer Synth-Ton 73–91. Ruhend: der Transient-Guardrail reißt ohne jede Änderung in 7–27 % und würde DSP-19 sichtbar machen. | Abweichung von der Spezifikation + Schwellenlücke (ruhend) | Künftig gälten gleichmäßige Flächen und saubere Synth-Töne als ständige „Einsätze" und verfälschten Vorher/Nachher- und Ursachenaussagen. | `eq-copilot/plugin/core/analysis/featureengine/Spektrum.h:585-608`, `:635-642`, `:679-708`; `docs/FL-Nakama-Sonden-Design-Entwurf.md:2618-2620`; [b11_fluss.py](skripte/gegenpruefung-statistik/b11_fluss.py) | neu |
| DSP-23 | **Kohärenz über die Bins eines Bandes summiert.** Eine reine Laufzeit dreht die Phase innerhalb des Bandes, die Summe löscht sich teilweise: 1 ms Versatz, voll kohärent, ergibt 0,48 bei 15,8 kHz (unter der Phasenschwelle 0,8 ab etwa 8,9 kHz). Zudem erreichen Hauptstufen-Bänder bei 44,1 kHz mit Blöcken bis 512 Samples nie die nötigen 8 Frames. Heute ohne Verbraucher. | Abweichung von der Spezifikation (ruhend) | Künftig bekäme ein nur 1 ms versetztes Stereopaar über etwa 9 kHz keine Phasen- oder Laufzeitaussage (sichere Seite, falsche Bezeichnung). | `eq-copilot/plugin/core/analysis/featureengine/Stereo.h:65-91`, `:225-244`; [b10_kohaerenz.py](skripte/gegenpruefung-statistik/b10_kohaerenz.py) | neu (NAK-177 betrifft nur den Bass) |
| DSP-24 | **K-Gewichtung um −0,04 LU versetzt.** Der Hochpass-Zähler ist auf Nyquist-Verstärkung normiert (b0 = 0,99504 statt 1,0 nach BS.1770-5 Tab. 2): ein 997-Hz-Sinus mit 0 dBFS liest −3,052 statt −3,01 LKFS; Versatz −0,038 bis −0,044 LU bei 44,1–192 kHz, 40 % des ±0,1-LU-Budgets. Die Validierung ist zirkulär (pyloudnorm mit denselben Parametern). | Härtung | Unhörbar; jede LUFS-Anzeige liegt 0,04 LU zu tief. | `eq-copilot/plugin/core/analysis/KGewichtung.h:80-94`; `eq-copilot/plugin/tests/GoldenTestMain.cpp:433`; [b7_kgewicht.py](skripte/gegenpruefung-pegel/b7_kgewicht.py) | teilweise NAK-199 |
| DSP-25 | **LRA mit 1 Hz statt mindestens 10 Hz abgetastet.** EBU Tech 3342 V4 §3.1 verlangt mindestens 10 Hz; der Kommentar verweist auf einen nicht existierenden §2.2. Normfälle 1–4 exakt; bei kurzen oder sprunghaften Passagen liegt das p95 der größten Abweichung bei etwa 0,9 LU. Heute ohne Anzeige. | Härtung | Heute nicht spürbar; eine künftige LRA-Anzeige wiche bei kurzen Passagen spürbar ab. | `eq-copilot/plugin/core/analysis/FeatureEngine.h:410-411`; `eq-copilot/plugin/core/analysis/featureengine/Lautheit.h:74-87`; [b8_lra.py](skripte/gegenpruefung-pegel/b8_lra.py) | neu |

## 4. Textwidersprüche aus der Planlesung (nicht einzeln gegengeprüft)

Vom Planleser gemeldet, an den genannten Zeilen gelesen, aber ohne eigenen
Gegenprüfer; zur Doku-Pflege.

- Der Master-EQ in Gen hat keinen festgelegten Messpunkt: ob Gens Befunde vor
  oder nach dem eigenen Master-EQ messen, steht nirgends (Entwurf :1156-1158).
- Das Livegitter heißt „64 perzeptive Bänder" (:2064, :2269) bzw. „32 bis 64"
  (:2668), die Datei ist eine gleichmäßige log-Gruppierung, keine ERB- oder
  Bark-Skala.
- EQ, Schutz-Zonen und Auto-Gain reichen von 20 Hz bis 20 kHz, die Analyse von
  30 Hz bis 18 kHz.
- Die Vorwärmzeit max(500 ms; 3·Release) (:2712, :3600) ignoriert das
  Ausschwingen der Filter (20 Hz, Q 24: τ ≈ 0,38 s, nach 500 ms noch etwa −11 dB).
- „Keine positive automatische Gesamtverstärkung" (:2825-2826) gegen den
  Auto-Gain-Deckel von +24 dB (`docs/plugin-wissen.md:790-793`).
- Die Ausrichtungsgrenzen fürs Delta (0,02 bis 0,5 Sample, :2556-2574) lassen
  bei 10 kHz −31,6 bis −3,8 dB Leckage durch; das Delta wäre im Hochton vom
  Ausrichtungsfehler dominiert.
- Die RBJ-Referenz mit vertagtem Decramping steht neben dem Anspruch eines
  vollwertigen EQ „wie am Markt" (:271-273).
- Die 3-dB-Grenzen stehen trotz Streichung weiter in mehreren Texten; das ist
  bereits als NAK-368 und NAK-369 registriert.

## 5. Prüfregeln aus dem Buch für die nächsten Goldens

Seitennamen relativ zu <https://www.dsprelated.com/freebooks/filters/>.

1. **Impulsantwort lang genug:** N > 7/(1−R_max) Samples für mehr als 60 dB
   Abklingen (`Frequency_Response_Matlab.html`). Goldens an den
   Parametergrenzen brauchen bis etwa 5 s (DSP-10).
2. **Zweiter, unabhängiger Rechenweg:** Jeder bilineare Entwurf erfüllt
   |H_d(f)| = |H_a(j·tan(πf/fs)/tan(πf0/fs))| (`Frequency_Warping.html`);
   nachgerechnet für alle sechs Typen auf ≤ 1,4e-7 dB. Taugt für Goldens,
   Auto-Gain und eine spätere Kurvenanzeige.
3. **Geschlossener Butterworth-Fall bei fs/4:** H(z) = (1+z⁻¹)² /
   ((2+√2) + (2−√2)z⁻²), −3 dB und −90° exakt
   (`Example_Second_Order_Butterworth_Lowpass.html`); RBJ trifft ihn auf 1e-16.
4. **Stabilitätsdreieck über den ganzen Parameterraum** mit Sicherheitsabstand:
   |a₂| < 1 und |a₁| < 1 + a₂ (`Step_Down_Procedure.html`), für alle Typen,
   20 Hz bis 0,45·fs, Q 0,15–24, ±12 dB, 44,1–192 kHz.
5. **Verschiebungs- und Blockinvarianz** bei statischen Koeffizienten: verzögerter
   Eingang gibt bitgleich verzögerte Ausgabe, jede Blockteilung dieselbe Ausgabe
   (`Showing_Linearity_Time_Invariance.html`).
6. **Minimalphase beweisen** per Cepstrum (`Matlab_listing_mps_m.html`): bei
   Glocken und Shelves Phasendifferenz 0,0000°; bei Notch und Cuts vorher die
   Nullstellen auf dem Einheitskreis abdividieren.
7. **Detektor exakt normieren:** Der RBJ-Bandpass mit 0-dB-Spitze hat für
   weißes Rauschen den Leistungsgewinn (1−a₂)/2 (`Constant_Peak_Gain_Resonator.html`);
   die analoge Näherung π/2·f0/Q weicht bei 5 kHz, Q 8 um 10 % ab.
8. **Dynamische Bänder sind nichtlinear** (`Nonlinear_Filter_Example_Dynamic.html`):
   Superpositionstests gelten dort nicht; Intermodulation und Klirr gehören in
   ihre Goldens.
9. **Bandbreite und Ausklingzeit aus den Polen rechnen**, nicht aus f0/Q: die
   Näherung R = e^{−πBT} gilt für matched-z und liegt bei bilinearen Entwürfen
   nahe Nyquist um den Faktor 5–9 daneben (`Relating_Pole_Radius_Bandwidth.html`).

## 6. Ergänzungsquellen (von der Quellenrecherche verifiziert)

| Thema | Quelle |
|---|---|
| RBJ-Formeln (Nakamas eigentliche Formelquelle) | Audio EQ Cookbook, W3C Working Group Note vom 08.06.2021, <https://www.w3.org/TR/audio-eq-cookbook/> |
| Decramping | M. Vicanek, „Matched Second Order Digital Filters", 2016, <https://www.vicanek.de/articles/BiquadFits.pdf>; S. J. Orfanidis, JAES 45(6):444–455, 1997 |
| TPT-SVF | A. Simper (Cytomic), <https://cytomic.com/files/dsp/SvfLinearTrapOptimised2.pdf>; V. Zavalishin, „The Art of VA Filter Design", Rev. 2.1.2 (2020) |
| Zeitvariante Filter, Glättung, Zipper-Messung | A. Wishnick, DAFx-14, <https://www.dafx14.fau.de/papers/dafx14_aaron_wishnick_time_varying_filters_for_.pdf>; J. Laroche, JAES 55(6):460–471, 2007 |
| EQ-Gesamtrahmen nach 2007 | V. Välimäki, J. D. Reiss, „All About Audio Equalization", Appl. Sci. 6(5):129, 2016, DOI 10.3390/app6050129 |
| Lautheit, True Peak, LRA | ITU-R BS.1770-5 (11/2023); EBU Tech 3341 und 3342, V4 (11/2023), <https://tech.ebu.ch/publications/tech3341> |
| Spektralanalyse, Fenster, Welch | J. O. Smith, „Spectral Audio Signal Processing", <https://www.dsprelated.com/freebooks/sasp/>; P. Welch, 1967, DOI 10.1109/TAU.1967.1161901; F. Harris, 1978, DOI 10.1109/PROC.1978.10837 |
| Bruchteil-Oktav-Bänder | IEC 61260-1:2014 |
| Vergleichsimplementierung | Faust `filters.lib`: `fi.svf` (Nesterov, nach Simper) und `fi.tf22t` taugen als Referenz; JOS' eigene `peak_eq_cq` und Shelves sind nicht RBJ-gleich |

## 7. Grenzen dieser Prüfung

- Gerechnet wurde an Nachbauten der Formeln, nicht am Binary. Die Nachbauten
  sind gegen die Quelle gelesen; ein Wert aus Tiefenaudit Phase 15 (+17,41 dB)
  ließ sich exakt reproduzieren.
- Kein Hörtest. Die Spalte „Wirkung" ist aus den Zahlen abgeleitet.
- §4 ist nicht einzeln gegengeprüft.
- Das Buch wurde in der DSPRelated-Fassung gelesen; Fußnoten fehlen dort, die
  gepflegte CCRMA-Fassung wurde wegen der robots.txt nicht gelesen.
