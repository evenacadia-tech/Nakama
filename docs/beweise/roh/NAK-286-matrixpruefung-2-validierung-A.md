# NAK-286 — Validierung der Codex-Matrixprüfung 2, Teil A (lesend, 14.09.2026)

Ticket NAK-286 (Plan S25e), Etappe 1. HEAD
`611979c7a7885947039ed93ff66f303c31238a5a`, Branch `master`. **Nur gelesen** —
nichts gebaut, kein Test, kein Skript, keine FL-Instanz, Git nur lesend;
geschrieben wurde ausschließlich diese Datei. Die untracked Ordner
`briefing-hub/` und `nimbalyst-local/` blieben unberührt.

Teil A prüft die **Befunde 1 bis 3** des Urteils
`docs/beweise/roh/NAK-286-matrixpruefung-2-611979c.txt` — die drei Befunde an
F-28 und M-66 bis M-68 (Reihenfolge wie in der Rohdatei).

**Zeilenversatz: null.** Das Urteil nennt seinen Prüfstand selbst
(`…-611979c.txt:17`: „HEAD blieb vor/nach der Prüfung identisch bei
`611979c7a7885947039ed93ff66f303c31238a5a`"), und `git status --short` zeigt
`docs/beweise/NAK-286.md` unverändert. Stichprobe: Urteil `:412` = die
Aufzählung „Vorbedingung der gewerteten Antwort" (HEAD 412), Urteil `:413` =
„Referenz über den Loop-Ausschnitt" (HEAD 413).

---

## Befund 1 — „Leite U_unten für phasenverschobene Zellen korrekt her"

**Wörtlich (Urteil), `docs/beweise/roh/NAK-286-matrixpruefung-2-611979c.txt:23-24`:**

> `[P2] Leite U_unten für phasenverschobene Zellen korrekt her —
> C:/Users/phili/Projekte/Nakama/docs/beweise/NAK-286.md:412-412`
> DEFEKT an F-28/M-67: Bei verschobenem Zellenraster ist die behauptete
> Untergrenze nicht gültig. Gegenbeispiel mit Energiegrenze G: 400 Quellzellen
> tragen abwechselnd die Halbzellenenergien (4G, 0) und (0, 4G); damit sind
> A=400 und W=0, also U_unten=39,8 s. Um 50 ms versetzt entstehen hingegen
> abwechselnd stille und aktive Engine-Zellen, insgesamt nur etwa 20 aktive
> Sekunden. `eq-copilot/plugin/src/AnalyseEngine.cpp:384–391` summiert die
> tatsächlichen Samples, nicht die Mittelwerte vollständiger Quellzellen. Der
> Abzug von Wechseln und zwei Randzellen begrenzt diesen Fehler nicht. Damit
> fehlt die von [P-5](NAK-286.md#L1033) verlangte gültige Herleitung;
> erforderlich ist eine samplegenaue Phasenprüfung oder eine tatsächlich
> gültige Schranke.

### Quelle an HEAD

**(1) Was die Engine wirklich zählt — `eq-copilot/plugin/src/AnalyseEngine.cpp:382-391`:**

```cpp
        // Aktivität §5.10.2 über die mittlere KANALenergie — der alte Mid-Mix
        // meldete Antiphase-Material als „still" (Paket B).
        zelleAktivEnergie += stereo ? 0.5 * (l * l + r * r) : l * l;
        if (++zellenStand >= zellenSamples)
        {
            loudness.zelle (zelleKEnergie);     // SONDE-008: fester Speicher statt push_back
            if (10.0 * std::log10 (zelleAktivEnergie / zellenSamples + 1e-30) > kAktivSchwelleDb)
                ++aktiveZellen;
            zelleKEnergie = zelleAktivEnergie = 0.0;
            zellenStand = 0;
```

mit `kAktivSchwelleDb = -60.0` (`:13`) und `zellenSamples = (int) std::lround
(0.1 * sr)` (`:270`). Die Entscheidung fällt also über den **arithmetischen
Mittelwert der tatsächlichen Samples einer Engine-Zelle**, nicht über die
Mittelwerte der Quellzellen, die sie überlappt. (Die von F-28 zitierte
Konstante heißt im Code `kAktivSchwelleDb`; `kAktivGateDb` ist die
gleichnamige −60,0-dB-Schwelle der `FeatureEngine`, `FeatureEngine.h:390`.)

**(2) Wo das Engine-Raster beginnt — `eq-copilot/plugin/src/AnalyseEngine.cpp:309`
(in `zuruecksetzen()`, `:277-336`):** `zellenStand = 0;` — das Raster startet
mit dem ersten Sample, das die Engine nach dem Rücksetzen sieht, und
`aktiveZellen = 0` (`:312`).

**(3) Welches Sample das ist — `eq-copilot/plugin/src/prozessor/Analyse.cpp:262-281`:**

```cpp
                    const auto grenzenVorher = merkmale.getrennteFenster();
                    const auto straddlesVorher = merkmale.straddleVerworfen();
                    if (merkmale.nimmBlock (*frei.block, frei.audio))
                        merkmalFrames.fetch_add (1);

                    const bool featureGrenze = merkmale.getrennteFenster() != grenzenVorher;
                    const bool blockVerworfen = merkmale.straddleVerworfen() != straddlesVorher;
                    if (featureGrenze)
                    {
                        engine.zuruecksetzen();
                        …
                    }
                    if (! blockVerworfen)
                    {
                        engine.verarbeite (frei.audio, …);
```

Der Block, in dem die Schleifengrenze liegen kann, wird **ganz verworfen**
(`eq-copilot/plugin/core/analysis/FeatureEngine.h:663-671`: „der Block selbst
geht nirgends ein"), und derselbe Block zieht die Grenze, die den Reset
auslöst. Das erste von der Engine verarbeitete Sample eines Umlaufs liegt
deshalb um bis zu eine Hostblocklänge **nach** dem Wrap. Probeeq macht es
gleich (`eq-copilot/plugin/sonde/SondeProcessor.cpp:592-604`).

**(4) Die Phase ist unbestimmt, nicht klein.** Nach F-28 ist die Quellzeit an
Hostposition p gleich p − v. Das Engine-Raster beginnt bei Hostposition δ
(δ ∈ (0, Blocklänge]), also bei Quellzeit δ − v. Die Phase gegen das
0,1-s-Raster der Quelle ist damit (δ − v) mod 0,1 s. `v` ist in F-19
vorzeichenbehaftet **in Frames** definiert und an nichts gebunden, was ein
Vielfaches von 4 410 Samples wäre — die Phase ist beliebig. Ein Argument über
die Blocklänge trägt die Schranke also nicht.

### Gegenstand

**F-28, HEAD-Zeile 412** (Vorbedingung, gekürzt):

> „**U_unten** = 0,1 s × (A(p_vor − v) − W(p_vor − v) − 2): die Engine zählt je
> 0,1 s eine Zelle, deren mittlere Kanalenergie über −60 dB liegt … A(p) ist
> die Zahl solcher Zellen der Quelle in [0, p) auf dem 0,1-s-Raster ab
> Songbeginn … W(p) die Zahl der Wechsel zwischen aktiver und nicht aktiver
> Zelle darin … **Warum diese Abzüge:** das Zellenraster der Engine beginnt
> beim Rücksetzen (`zellenStand = 0`, `AnalyseEngine.cpp:309`), nicht auf dem
> Raster der Quelle; **eine Engine-Zelle zwischen zwei aktiven Quellzellen ist
> aktiv und zwischen zwei nicht aktiven nicht aktiv, weil sich die Energie
> mittelt** (`:384`, `:388`); anders zählen kann sie nur an einem Wechsel und
> je einmal an der angeschnittenen ersten Zelle des Umlaufs (Anlauf) und an der
> letzten."

**M-67, HEAD-Zeile 599:** „`snapshot.zustand` = `messbereit`; **U_unten ≤
`snapshot.aktiv_sekunden` ≤ U_oben = 45,6 s (Rechnung F-28)**" — und als
Urteilsweg „VERFEHLT (Vorbedingung oder Band, mit Zuordnung)". Die Vorbedingung
ist ausdrücklich **kein** `roh`-Feld: sie löst Szenario-Exit 4 aus (F-28
Zeile 412: „sonst verfehlt „Vorbedingung", Szenario-Exit 4").

### Status: BESTÄTIGT — und schärfer als das Urteil

1. **Die tragende Zwischenbehauptung ist falsch.** „Eine Engine-Zelle zwischen
   zwei aktiven Quellzellen ist aktiv, weil sich die Energie mittelt" gilt nur,
   wenn die Energie **innerhalb** einer Quellzelle homogen ist. Die Engine
   mittelt nach `:384-388` über die tatsächlichen Samples ihres eigenen
   Fensters; eine Engine-Zelle, die den energiearmen Schwanz der einen und den
   energiearmen Kopf der nächsten Quellzelle überlappt, kann unter dem Gate
   liegen, obwohl beide Quellzellen darüber liegen. Die Annahme steht in F-28
   nicht als Annahme, sondern als Begründung („weil sich die Energie mittelt")
   — sie ist in der Feinheit selbst nicht hergeleitet und an keiner Quelle
   gemessen.
2. **Das Gegenbeispiel rechnet an der Engine-Formel nach.** Sei G die
   Energiegrenze (mittlere Kanalenergie zu −60 dB, also 10⁻⁶). 400 Quellzellen
   mit den Halbzellenenergien (4G, 0) bzw. (0, 4G) haben je den Zellmittelwert
   2G > G: A = 400, W = 0, U_unten = 0,1 s × (400 − 0 − 2) = **39,8 s**. Bei
   Phase 50 ms überlappt jede Engine-Zelle die zweite Hälfte der einen und die
   erste Hälfte der nächsten Quellzelle; die Mittelwerte alternieren zwischen
   0 und 4G, also sind rund 200 von 399 Zellen aktiv: **≈ 20,0 s**. Der Wert
   des korrekten Baus liegt um fast die Hälfte unter der Schranke. Die Rechnung
   des Prüfers ist an `:384-389` nachvollziehbar und stimmt.
3. **Die Abzüge treffen den Fehler nicht.** W(p) fängt Wechsel ab, die beiden
   Randzellen den Anlauf und die letzte angeschnittene Zelle. Der Phasenfehler
   sitzt dagegen an **jeder** Zelle des Umlaufs; im Gegenbeispiel ist W = 0.
4. **Verschärfung: F-28 widerspricht sich in derselben Aufzählung.** Für die
   Rasterspanne S_LUFS begründet Zeile 413 ausdrücklich „weil das Blockraster
   der Engine gegen das der Quelle **beliebig** liegt" und nimmt deshalb die
   Spannweite über alle 4 410 Rasterversätze. Genau diese Beliebigkeit
   bestreitet die Herleitung von U_unten eine Aufzählung höher. Beide Sätze
   können nicht zugleich gelten.
5. **Verschärfung: eine zweite, unbenannte Annahme.** Die Engine beginnt einen
   Umlauf erst mit dem ersten Block nach dem verworfenen Straddle-Block
   (Quelle 3). U_unten zählt dagegen A(p) ab **Quellbeginn 0**. Der Abzug „zwei
   Randzellen" deckt das nur, solange eine Hostblocklänge unter 0,1 s bleibt;
   die Blocklänge ist in Matrix, Szenarien und Bereitschaftslauf nirgends
   gemessen oder festgelegt.
6. **Wirkung:** ein korrekt gebautes Plugin kann die Vorbedingung verfehlen und
   `snapshot-runde01` auf Szenario-Exit 4 werfen. Das ist dieselbe Klasse wie
   Matrixprüfung 1 Befund 1 (≥ 60 aktive Sekunden) und Befund 5 („Δ aktiv ≤
   Δ gesamt wirft einen korrekten Bau auf Exit 4"), §10.1.

### Einordnung (Vorschlag): DEFEKT

U_unten ist ein Abnahmekriterium mit Exit 4 (F-28 Zeile 412, M-67 Zeile 599),
seine Herleitung steht in F-28 und ist **nicht gültig**: sie stützt sich auf
eine nicht hergeleitete Homogenitätsannahme innerhalb der Quellzelle. Nach
§10.2 **P-5** („Eine Toleranz, die nicht aus Messdaten oder einer gültigen, in
F-28 mit Quelle notierten Abschätzung hergeleitet ist, ist kein
Abnahmekriterium") darf sie kein Exit 4 auslösen. Zusätzlich verletzt sie den
Leitsatz aus §10.3, weil die Zeile mit korrektem Bau rot werden kann.

### Schließende Änderung (Vorschlag)

1. **Gültige Schranke statt Mittelungsargument.** U_unten wird in Etappe 4
   samplegenau **über alle Phasenlagen** gerechnet, mit demselben Werkzeug und
   demselben SHA-256 der Quelle wie A und W: für jeden Rasterversatz φ ∈ {0 …
   4 409 Samples} die Zahl der Zellen des Ausschnitts [φ, p_vor − v), deren
   Mittelwert nach der Formel aus `:388` über dem Gate liegt; U_unten = 0,1 s ×
   (Minimum über alle φ) − 0,1 s je verworfenem Kopfblock. Das ist dieselbe
   Konstruktion, die F-28 für S_LUFS schon führt (4 410 Rasterversätze), und
   damit ohne neue Annahme.
2. **Bis diese Rechnung mit Quelle in F-28 steht: `roh`.** U_unten löst dann
   kein Exit 4 aus (P-5); die Vorbedingung besteht in der Zwischenzeit nur aus
   `metrics_version`, `zustand = messbereit` und U_oben (die Obergrenze ist von
   der Phase unberührt: mehr als ein Umlauf Zellen kann nur entstehen, wenn
   über den Wrap integriert wurde).
3. **Blocklänge benennen.** Der Kopfverlust eines Umlaufs (verworfener
   Straddle-Block plus Rasterphase) wird in F-28 als eigener Term geführt und
   die Hostblocklänge im Bereitschaftslauf roh mitgeschrieben, statt still
   unter „zwei Randzellen" zu verschwinden.
4. **Rotbeweis an der Zusage.** Der Selbsttest `snapshot_messpunkt_und_baender`
   (M-67) bekommt eine Attrappenfolge mit phasenverschobenem Zellenraster: die
   Schranke aus der Minimumrechnung muss sie annehmen, die Schranke aus der
   Mittelungsrechnung sie verwerfen.

---

## Befund 2 — „Verschiebe bei negativem v auch den Referenzbeginn"

**Wörtlich (Urteil), `docs/beweise/roh/NAK-286-matrixpruefung-2-611979c.txt:26-27`:**

> `[P2] Verschiebe bei negativem v auch den Referenzbeginn —
> C:/Users/phili/Projekte/Nakama/docs/beweise/NAK-286.md:413-413`
> DEFEKT an F-28/M-66 bis M-68: Für den ausdrücklich vorgesehenen negativen
> Versatz, etwa v=−75 600 Frames aus M-75, hört der Host ab Position 0 bereits
> die Quelle ab 1,714286 s. Seine Referenz ist deshalb nicht das hier
> festgelegte Präfix [0, p−v), sondern ein Ausschnitt mit ebenfalls
> verschobenem Beginn. Zusätzlich verlangt eine zulässige späte
> Positionsklammer Tabellenwerte jenseits von 45,696 s, obwohl R_X nur bis zum
> Loopende aufgebaut wird. Dadurch vergleicht das fortgesetzte
> Snapshot-Szenario mit nicht gehörtem Material beziehungsweise außerhalb
> seiner Referenztabelle. Der nach [P-4](NAK-286.md#L1032) unterstützte
> negative Versatz muss bei beiden Ausschnittgrenzen und beim Tabellenbereich
> berücksichtigt werden.

### Quelle an HEAD

**(1) Das Vorzeichen von v ist in F-19 nicht definiert — HEAD-Zeile 386:**

> „**Versatzsuche über die volle Songlänge** (§10.2 P-4): Versatz v
> vorzeichenbehaftet in Frames, |v| < N; die Kreuzkorrelation beider Kanäle
> über die N Frames liefert den Kandidaten, der Vergleich nach Ausrichtung um v
> über die N − |v| überlappenden Frames entscheidet."

Mehr steht nirgends: `git grep -n "Versatz"` über das Manifest trifft F-17,
F-19, F-26, F-28, M-62, M-63, M-75, §5.7 Punkt 3, §10.2 P-4 und §11.1 Punkt 4 —
keine Stelle sagt, ob ein positives v heißt „der Render hinkt der Quelle nach"
oder „der Host hört die Quelle früher". F-28 verwendet dasselbe v ohne
Vorzeichenregel: „v der Versatz aus `ergebnis.json` in Sekunden (F-19)".

**(2) Negatives v ist ausdrücklich vorgesehen — M-75, HEAD-Zeile 589:**

> „Urteil VERSATZ mit v (vorzeichenbehaftet, in Frames) und 0 Abweichungen über
> die N − |v| überlappenden Frames … künftig Selbsttest
> `versatz_ist_voraussetzung` (Attrappenpaare mit **v = ±1 und v = ±75 600
> Frames** = ein Takt bei 140 BPM: 4 × 60/140 s × 44 100 Hz, §0.5 Punkt 1)"

75 600 / 44 100 = **1,714286 s** — die Zahl des Prüfers stimmt, und das
Projekt läuft nach §0.5 Punkt 1 mit 140 BPM (Takt 1,714 s).

**(3) Der Tabellenbereich endet am Loopende — F-28, HEAD-Zeile 413:** „Die
ersten **2 015 193 Frames** der Quelle (0 bis 45,696 s, F-19) als
Ausschnittdatei … Dieselbe Rechnung je 0,1-s-Rasterpunkt p über [0, p) ergibt
die Tabellen R_X(p)."

**(4) Die Quelle ist länger als der Loop — §0.4, HEAD-Zeile 213-216:**
`Testtrack.wav` hat 5 470 096 Frames = 124,04 s, davon nach
`KALIBRIER-PROTOKOLL.md:66` 77,7 s Stille. Material jenseits von 45,696 s
existiert also physisch, ist aber in R_X nicht enthalten — ein Loop-Ausschnitt,
der die Quelle früher anschneidet, hört am Ende genau dieses Material.

**(5) Die Klammer darf spät liegen — F-28, HEAD-Zeile 411:** „Gewertet wird je
Rolle **die letzte Antwort mit intakter Klammer** (p_vor ≤ p_nach < L,
L = 45,696 s)". Mit p_nach knapp unter L und |v| = 1,714 s verlangt
⌈(p_nach − v)/0,1 s⌉ Rasterpunkte bis 47,4 s.

### Gegenstand

**F-28, HEAD-Zeile 413** (Referenz, gekürzt):

> „**Referenz über den Loop-Ausschnitt** … Die ersten 2 015 193 Frames der
> Quelle (0 bis 45,696 s, F-19) als Ausschnittdatei … Dieselbe Rechnung je
> 0,1-s-Rasterpunkt p über [0, p) ergibt die Tabellen R_X(p). **Referenz einer
> Antwort ist die Spanne [min, max] von R_X über die Rasterpunkte von
> ⌊(p_vor − v) / 0,1 s⌋ bis ⌈(p_nach − v) / 0,1 s⌉** (der Snapshot entsteht
> zwischen beiden Klammerpositionen)."

**F-27, HEAD-Zeile 409 und M-66, HEAD-Zeile 598:** „Δ_K = 0,0 dB exakt (nach
Ausrichtung wertgleich; **v verschiebt nur den Messpunkt**, F-28)".

**M-67/M-68, HEAD-Zeilen 599-600:** die Bänder `R_LUFS + Δ_K ± (0,07 LU +
S_LUFS)`, `R_TP + Δ_K ± 0,12 dB`, `R_width ± 0,01`, `R_corr ± 0,01`,
`R_low_frac ± 0,02` und die Resonanz-Teilmenge lesen alle dieselbe Tabelle
über dieselben Rasterpunkte.

### Status: BESTÄTIGT — mit einer Präzisierung am Vorzeichen

1. **Die Physik ist richtig beschrieben.** Hört der Host an Position 0 bereits
   Quellzeit a > 0 (der Clip ist nach vorn geschoben oder vorn beschnitten),
   dann hat die Engine an Position p den Quellausschnitt **[a, p + a)**
   integriert. Das ist kein Präfix ab 0: R_X(p') beschreibt aber
   ausschließlich Präfixe [0, p'). Der Referenzwert gehört damit zu einem
   Materialausschnitt, den das Plugin nie gehört hat — bei LUFS-I, width, corr
   und low_frac sind das energiegewichtete Größen über den ganzen Ausschnitt,
   der Fehler ist nicht der einer Randzelle.
2. **Der Tabellenüberlauf folgt zwingend.** Mit demselben Vorzeichen ist die
   obere Grenze p + |v|; für die zulässige späte Klammer (p_nach knapp unter
   45,696 s) liegt sie außerhalb von R_X, das nur bis 45,696 s gebaut wird
   (Quelle 3). Das Szenario hat dann für seine gewertete Antwort **gar keinen**
   Referenzwert — F-24 kennt dafür nur „fehlende Rechnung aus F-28" mit
   Szenario-Exit 5, also fällt ein regulärer Lauf mit gültigem VERSATZ-Urteil
   in die Voraussetzung statt in die Messung.
3. **Präzisierung: das Vorzeichen ist gar nicht festgelegt.** F-19 sagt nur
   „vorzeichenbehaftet"; welche Richtung negativ heißt, steht nirgends
   (Quelle 1). Die Formel `p − v` ist genau für **eine** der beiden möglichen
   Konventionen richtig (Render hinkt nach: gehört wurde effektiv das Präfix
   [0, p − |v|), innerhalb der Tabelle) und für die andere falsch (Host hört
   früher: gehört wurde [|v|, p + |v|), außerhalb der Tabelle). Der Defekt
   besteht also unabhängig davon, welche Konvention `nulltest.py` später wählt
   — und zusätzlich fehlt die Festlegung, an der man es prüfen könnte. Der
   Prüfer nennt die Lage, die bricht, aber begründet sie mit einer
   Vorzeichenannahme, die das Manifest nicht trägt.
4. **Verschärfung: bei der brechenden Konvention stimmt auch F-27 nicht mehr.**
   „v verschiebt nur den Messpunkt" ist die Aussage, aus der Δ_K = 0,0 dB
   „exakt" gilt. Verschiebt sich zusätzlich der Referenz**beginn** und läuft
   das Ende in den Stillebereich der Quelle (§0.4: 77,7 s Stille), verschiebt v
   nicht nur den Messpunkt, sondern den gehörten Ausschnitt — dieselbe Zeile
   trägt beide Aussagen und kann nur eine halten.

### Einordnung (Vorschlag): DEFEKT, mit einem Lückenanteil am Vorzeichen

DEFEKT an F-28 (Referenzdefinition) und mittelbar an M-66 bis M-68, die diese
Referenz lesen: die Zusage „v verschiebt nur den Messpunkt" trägt für eine der
beiden Vorzeichenlagen nicht, und die Bänder werden gegen Tabellenwerte
geprüft, die entweder nicht gehörtes Material beschreiben oder nicht
existieren. Der **Lückenanteil**: F-19 legt das Vorzeichen von v nicht fest,
obwohl M-75, F-27 und F-28 damit rechnen — die Regeln schweigen an der Stelle,
an der die Rechnung entschieden würde (Muster wie §10.1 Befund 4).

### Schließende Änderung (Vorschlag)

1. **Vorzeichen in F-19 wörtlich festlegen** und in M-75 spiegeln, zum Beispiel:
   „v > 0 heißt, der Render trägt die Quelle um v Frames **später**; v < 0
   heißt, der Host hört an Position 0 bereits Quellzeit |v|." Der Selbsttest
   `versatz_ist_voraussetzung` prüft beide Vorzeichen an Attrappen, deren
   Richtung aus der Konstruktion bekannt ist (er tut es mit ±1 und ±75 600
   schon, ohne die Richtung zu benennen).
2. **Referenz als Ausschnitt statt als Präfix.** F-28 rechnet R_X nicht mehr
   über [0, p), sondern über den **tatsächlich gehörten** Ausschnitt
   [a, a + T), mit a = Quellzeit an Hostposition 0 und T = gehörte Dauer; für
   a = 0 fällt das auf die heutige Präfixrechnung zurück. Zwei Grenzen, zwei
   Rasterpunkte — das ist dieselbe Tabellenrechnung mit einem zusätzlichen
   Argument.
3. **Tabellenbereich an den Ausschnitt binden.** Die Ausschnittdatei umfasst
   [a, a + L) statt [0, L); ihr SHA-256 und a stehen roh in der Rohzeile. Reicht
   der Ausschnitt über die Quelle hinaus, ist das eine Szenario-Voraussetzung
   mit gemessenem Grund, nie eine stille Bereichsüberschreitung.
4. **F-27 nachziehen:** Δ_K = 0,0 dB gilt bei VERSATZ weiterhin (nach
   Ausrichtung wertgleich), aber der Satz „v verschiebt nur den Messpunkt" wird
   durch „v verschiebt Beginn und Ende des gehörten Ausschnitts; die Referenz
   wird auf denselben Ausschnitt gerechnet" ersetzt.
5. **Rotbeweis an der Zusage:** eine Attrappe mit v < 0 und später Klammer muss
   ohne die Ausschnittrechnung an der Tabellengrenze scheitern (rot) und mit
   ihr die Bänder treffen.

---

## Befund 3 — „Richte die Referenz am publizierten Messstand aus"

**Wörtlich (Urteil), `docs/beweise/roh/NAK-286-matrixpruefung-2-611979c.txt:29-30`:**

> `[P2] Richte die Referenz am publizierten Messstand aus —
> C:/Users/phili/Projekte/Nakama/docs/beweise/NAK-286.md:413-413`
> DEFEKT an F-28/M-67/M-68: Eine Antwort zwischen zwei Positionsabfragen
> enthält nicht zwingend Messwerte dieses Zeitfensters. Der weiterhin
> vorgesehene Aufruf `engine.snapshot()` liefert lediglich `fertig`
> (`eq-copilot/plugin/src/AnalyseEngine.cpp:1123–1126`); integrierte Lautheit,
> Stereo- und Spektralwerte werden bei der schweren Auswertung aktualisiert
> (`:1059–1066`), deren Kadenz 250 ms beträgt
> (`eq-copilot/plugin/src/WorkerCadence.h:28–29`). Bei letzter Publikation um
> 45,0 s und Antwortklammer 45,19–45,22 s umfasst die Referenz nur
> Rasterpunkte 45,1–45,3 s, nicht den tatsächlichen Messstand. Die leichte
> Publikation behebt das nicht (`AnalyseEngine.cpp:787–797`). Für ein gültiges
> Band nach [P-5](NAK-286.md#L1033) muss die Referenz die Materialzeit
> beziehungsweise nachgewiesene Publikationslatenz berücksichtigen, statt
> JSON-Lesezeit mit Messzeit gleichzusetzen.

### Quelle an HEAD

**(1) `snapshot()` gibt den zuletzt publizierten Stand —
`eq-copilot/plugin/src/AnalyseEngine.cpp:1123-1127`:**

```cpp
MessSnapshot AnalyseEngine::snapshot() const
{
    std::lock_guard<std::mutex> l (snapMutex);
    return fertig;
}
```

`fertig` (`AnalyseEngine.h:364`) wird an genau drei Stellen geschrieben:
`zuruecksetzen()` (`:326-334`), `auswertenLeicht()` (`:796-797`) und
`auswerten()` (`:1116-1120`).

**(2) Die schwere Auswertung füllt die Bandfelder —
`eq-copilot/plugin/src/AnalyseEngine.cpp:1059-1066`:**

```cpp
    fuelleBasis (s);   // Zustand, Sekunden, Live-Kurve, TP/Crest/Kurz-LUFS

    finalisiereLoudness (s);
    finalisiereSkalar (s);
    finalisiereLtas (s);
    berechneAbdeckung (s);
    berechnePerzentile (s);
    findeResonanzen (s);
```

`finalisiereLoudness` setzt `lufsIntegriert` (`:702-707`); `lufs_integriert`,
`width`, `corr`, `low_frac`, `centroid_mag_hz` und `resonanzen` — also **alle**
Felder mit Band in M-67/M-68 außer True Peak — entstehen nur hier.

**(3) Die leichte Publikation deckt genau die anderen Felder —
`eq-copilot/plugin/src/AnalyseEngine.cpp:787-798` und `fuelleBasis`
(`:741-785`):** `samplerate`, `verarbeiteteSamples`, `nanErsetzt`,
`gesamtSekunden`, `aktivSekunden`, `zustand`, `ltasLiveDb`, `truePeakDb`,
`crestDb`, `lufsShort` — dokumentiert an `AnalyseEngine.h:196-199`
(„aktualisiert **NUR** Live-Kurve, Kurz-LUFS, True Peak, Crest und Zustand im
veröffentlichten Snapshot").

**(4) Kadenz — `eq-copilot/plugin/src/WorkerCadence.h:28-29`:**

```cpp
    static constexpr auto leichtesIntervall = std::chrono::milliseconds (50);
    static constexpr auto schweresIntervall = std::chrono::milliseconds (250);
```

Die Deadline wird nach dem Feuern **von jetzt aus** neu gesetzt (`:51`), der
Workerzug wartet je Runde bis zu 50 ms
(`eq-copilot/plugin/src/prozessor/Analyse.cpp:355-356`), und die schwere
Auswertung entfällt, solange eine Steueranfrage ansteht (`:293-295`: „Die
Deadline wird dann nicht verbraucht, sondern nach der Uebergabe im naechsten
Workerzug bedient"). 250 ms ist damit die **Untergrenze** des Abstands, nicht
die Obergrenze der Publikationslatenz; dazu kommen Queue-Rückstau und die
Blocklänge zwischen Audio-Thread und Worker.

**(5) Der Briefkastentakt liest höchstens sekündlich — F-12, HEAD-Zeile 372:**
`juce::Timer` im Abstand 1 000 ms, je Takt eine Existenzprüfung (F-14). Die
Antwort entsteht also irgendwann innerhalb der Klammer; ihr Inhalt stammt aus
einer Publikation, die **vor** diesem Zeitpunkt lag.

### Gegenstand

**F-28, HEAD-Zeile 413:** „Referenz einer Antwort ist die Spanne [min, max] von
R_X über die Rasterpunkte von ⌊(p_vor − v) / 0,1 s⌋ bis ⌈(p_nach − v) / 0,1 s⌉
(**der Snapshot entsteht zwischen beiden Klammerpositionen**)."

**F-28, HEAD-Zeile 411:** „p_vor vor dem Schreiben der Anfrage, p_nach nach der
letzten erwarteten Antwort."

**M-67/M-68, HEAD-Zeilen 599-600:** alle Bänder werden gegen diese Spanne
geprüft; ein Fehlschlag geht nach der Zuordnungsregel an Messung, Referenz oder
Vergleichsbasis (Szenario-Exit 4).

### Status: BESTÄTIGT — und in zwei Punkten schärfer

1. **Die Klammer misst die falsche Uhr.** p_vor und p_nach klammern korrekt den
   Zeitraum, in dem die **Antwortdatei** entsteht. Der Inhalt der Antwort ist
   der letzte publizierte Stand (Quelle 1). Zwischen der Materialzeit dieses
   Standes und p_vor liegt mindestens ein schweres Intervall (250 ms), im
   ungünstigen Fall mehr (Quelle 4). Die Spanne beginnt aber erst bei p_vor —
   der wahre Bezugspunkt kann vollständig **links** davon liegen. Das
   Zahlenbeispiel des Prüfers (Publikation 45,0 s, Klammer 45,19–45,22 s,
   Rasterpunkte 45,1–45,3 s) rechnet die Formel korrekt nach.
2. **P-5 ist einschlägig.** Die Bänder ± 0,07 LU, ± 0,12 dB, ± 0,01 und ± 0,02
   sind gemessene Mess-Integritäten zwischen offline und Plugin **auf demselben
   Material**; sie decken keinen Materialversatz. Ein Band, dessen Mitte an
   einer anderen Materialzeit hängt als die Messung, ist keine hergeleitete
   Toleranz mehr.
3. **Verschärfung 1: die Antwort ist in sich zeitlich uneinheitlich.** Nach
   einer leichten Publikation trägt derselbe `fertig`-Stand `aktiv_sekunden`,
   `gesamt_sekunden`, `zustand` und `true_peak_dbtp` aus der Materialzeit T,
   aber `lufs_integriert`, `width`, `corr`, `low_frac`, `centroid_mag_hz` und
   `resonanzen` aus T − bis zu 250 ms (Quellen 2 und 3). M-67 prüft die
   **Vorbedingung** (U_unten ≤ `aktiv_sekunden` ≤ U_oben) also an einer
   frischeren Materialzeit als die Bänder derselben Antwort. Eine einzige
   Referenzspanne für beides ist damit strukturell falsch, unabhängig von der
   Latenz. Das berührt zugleich die Invariante „Engine … liefert **kohärente**
   Mess-Snapshots" (`CLAUDE.md`): kohärent ist der Stand nur innerhalb einer
   Feldgruppe.
4. **Verschärfung 2 — für Probeeq gibt es an HEAD überhaupt keinen
   publizierten Stand.** `auswerten()` und `auswertenLeicht()` werden im ganzen
   Pluginbaum nur an zwei Stellen gerufen:
   `eq-copilot/plugin/src/prozessor/Analyse.cpp:302` und `:307` — im Worker von
   **Gen**, auf dessen eigener Engine. Der Probeeq-Worker
   (`eq-copilot/plugin/sonde/SondeProcessor.cpp:592-645`) ruft auf seiner
   `analyseEngine` (`SondeProcessor.h:402`) nur `zuruecksetzen()` (`:586`,
   `:599`), `verarbeite()` (`:603`) und `lautheitFuerTelemetrie()` (`:610`).
   `fertig` bleibt dort für immer der Stand aus `zuruecksetzen()`, also
   `zustand = MessZustand::keineDaten` und `aktivSekunden = 0.0`
   (`AnalyseEngine.h:69`, `:89-92`; `AnalyseEngine.cpp:326-334`). F-5
   (HEAD-Zeile 358) schreibt aber vor: „Probeeq baut es aus
   `analyseEngine.snapshot()`". Der bestehende Snapshotweg beantwortet genau
   diesen Zustand mit „noch keine Messdaten"
   (`eq-copilot/plugin/src/prozessor/Analyse.cpp:1270-1275`), und F-4 hat den
   Grundcode `noch_keine_messdaten` dafür. **Mit dem in F-5 vorgesehenen Bau
   erreicht die Rolle Probeeq die Vorbedingung von M-67 nie**, und M-68 liest
   nie ein Band. Kein Eintrag in §3 (Etappe 2), F-5, F-15 oder der Matrix baut
   eine Publikation im Probeeq-Worker. Das ist derselbe Fehler wie
   Matrixprüfung 1 Befund 1: die Zeile scheitert mit korrektem Bau.

### Einordnung (Vorschlag): DEFEKT (zwei Teile)

- **(a) Referenz an der Lesezeit statt an der Materialzeit** — DEFEKT an F-28
  Zeile 413 und damit an M-67/M-68: die Bänder sind nach §10.2 P-5 nicht
  hergeleitet, solange die Referenzspanne die Publikationslatenz und die
  Trennung leichter/schwerer Felder nicht trägt.
- **(b) Kein publizierter Messstand in Probeeq** — DEFEKT an F-5 und M-67/M-68
  (der Beweisweg trägt die Zusage nicht). Für den Bauplan §3/Etappe 2 ist es
  zugleich eine Lücke: die Pfade der Etappe 2 enthalten die Stelle nicht.

### Schließende Änderung (Vorschlag)

1. **Materialzeit statt Wanduhr als Anker.** Die Antwort trägt bereits
   `gesamt_sekunden` (`eq-copilot/plugin/src/prozessor/Analyse.cpp:1304`;
   Schema `eq-copilot/schemas/eq-snapshot.schema.json:54`) = seit dem letzten
   Rücksetzen verarbeitete Samples ÷ Samplerate, also die **Materialdauer des
   laufenden Umlaufs**. F-28 klammert die Referenz an dieser Größe statt an
   p_vor/p_nach: Rasterpunkt = Quellbeginn des Umlaufs + `gesamt_sekunden`
   (+ Kopfverlust aus Befund 1). Die Positionsklammer bleibt, aber nur noch als
   Plausibilitätsprüfung („die Antwort gehört in diesen Umlauf") und roh.
2. **Zwei Anker, weil es zwei Feldgruppen gibt.** Damit die Bandfelder ihren
   eigenen Anker haben, publiziert `auswerten()` zusätzlich die Materialzeit
   ihres eigenen Standes (ein additives Feld neben `gesamt_sekunden`, das
   `auswertenLeicht()` nicht überschreibt), und der Umschlag reicht es nach
   F-4/F-6 durch. Die leichten Felder (Vorbedingung, True Peak) hängen an
   `gesamt_sekunden`, die schweren an diesem Feld. Additiv wie P-6, kein
   bestehendes Feld, kein Audio-Thread-Zugriff.
3. **Ersatzweg ohne neues Feld:** ist der Zusatz unerwünscht, klammert F-28 von
   ⌊(p_vor − v − L_pub)/0,1 s⌋ bis ⌈(p_nach − v)/0,1 s⌉ mit einer **an der
   Quelle hergeleiteten** oberen Latenzschranke L_pub (schweres Intervall
   250 ms + Wartefrist des Workerzugs 50 ms + Blocklänge + Queue-Rückstau,
   jeder Summand mit Stelle); bis diese Herleitung in F-28 steht, sind die
   Bänder nach P-5 `roh`. Der Ankerweg (1)/(2) ist vorzuziehen, weil er ohne
   Schätzung auskommt.
4. **Publikation im Probeeq-Worker bauen.** Etappe 2 bekommt im
   `SondeProcessor`-Worker dieselbe `WorkerKadenz` wie Gen
   (`eq-copilot/plugin/src/WorkerCadence.h`), die nach dem Verarbeiten
   `analyseEngine.auswerten()` beziehungsweise `auswertenLeicht()` ruft — nur
   im Worker, nie im Briefkastentakt (Single-Writer-Kontrakt,
   `AnalyseEngine.h:192-201`), kein Zugriff auf dem Audio-Thread, Nulltest
   A1/A16 und Goldens bytegleich (Pfadfreigabe wie P-6). F-5 nennt die Stelle;
   eine neue Matrixzeile (Etappe 2, Bein B30) trägt die Zusage „Probeeq
   veröffentlicht seinen Messstand" mit dem Rotbeweis: Publikation entfernt →
   die Antwort trägt `snapshot` null mit Grund `noch_keine_messdaten` → rot.
5. **Rotbeweis für (a):** die Attrappenfolge in `snapshot_messpunkt_und_baender`
   bekommt eine Antwort, deren Messstand ein schweres Intervall älter ist als
   p_vor; ohne Anker muss sie das Band verfehlen, mit Anker treffen.

---

## Gemeinsame Ursache

Der Prüfer sieht in allen drei Befunden dieselbe Ursache: **Referenz und
Messung sind nicht auf dieselbe Materialzeit bezogen.** Das wird **bestätigt**,
und es ist die Fortsetzung der Ursache (a) aus §10.3 („Engine-Zustand als
kumulativ gelesen") auf der Referenzseite: dort wurde geprüft, ob eine Zeile
einen Engine-Wert über eine Grenze hinweg liest; hier fehlt die Gegenprobe, ob
die **Referenz** genau das Material beschreibt, das die Engine bis zur
Publikation integriert hat.

Vier Glieder derselben Kette, alle in F-28 Zeilen 411-413:

| Glied | Wo die Materialzeit auseinanderläuft | Befund |
|---|---|---|
| Phase des Zellenrasters | Engine-Zellen liegen beliebig gegen das 0,1-s-Raster der Quelle; U_unten rechnet auf dem Quellraster | 1 |
| Kopf des Umlaufs | die Engine beginnt erst mit dem ersten Block nach dem verworfenen Straddle-Block, die Referenz bei Quellzeit 0 | 1 (Verschärfung) |
| Versatz v | Beginn **und** Ende des gehörten Ausschnitts verschieben sich, die Referenz bleibt ein Präfix ab 0 | 2 |
| Publikationslatenz | die Antwort trägt einen bis zu ein schweres Intervall älteren Stand, und leichte und schwere Felder tragen verschiedene Materialzeiten | 3 |

Daraus folgt ein Maßstab für die Nacharbeit, nicht nur drei Punktkorrekturen:
**jede Zahl in F-28, die ein Band oder eine Vorbedingung trägt, nennt die
Materialzeit, auf die sie sich bezieht, und die Quelle, aus der diese
Materialzeit gelesen wird.** Wo diese Materialzeit nicht aus dem Umschlag
selbst lesbar ist, bleibt der Wert nach §10.2 P-5 `roh`.

---

## Schlusstabelle

| Befund | Status | Einordnung (Vorschlag) | Schließende Änderung (Kurz) |
|---|---|---|---|
| 1 — U_unten für phasenverschobene Zellen (F-28 Zeile 412, M-67) | BESTÄTIGT, schärfer: die Begründung „weil sich die Energie mittelt" setzt Homogenität innerhalb der Quellzelle voraus; das Gegenbeispiel (39,8 s Schranke gegen ≈ 20 s Messwert) rechnet an `AnalyseEngine.cpp:384-389` nach; F-28 begründet die Rasterspanne S_LUFS eine Zeile später mit genau der Beliebigkeit, die es hier bestreitet | **DEFEKT** (Abnahmekriterium mit Exit 4 ohne gültige Herleitung, §10.2 P-5; korrekter Bau kann rot werden, §10.3 Leitsatz) | U_unten als Minimum über alle 4 410 Rasterversätze samplegenau rechnen (dieselbe Konstruktion wie S_LUFS), Kopfverlust je Umlauf als eigener Term, Hostblocklänge roh mitschreiben; bis die Rechnung mit Quelle in F-28 steht `roh`, kein Exit 4; Rotbeweis mit phasenverschobener Attrappe |
| 2 — Referenzbeginn bei negativem v (F-28 Zeile 413, M-66 bis M-68) | BESTÄTIGT, präzisiert: der gehörte Ausschnitt ist [a, p+a), kein Präfix, und die späte Klammer verlangt Tabellenwerte über 45,696 s hinaus; **das Vorzeichen von v ist in F-19 aber gar nicht festgelegt**, die Formel `p − v` trägt nur eine der beiden möglichen Konventionen | **DEFEKT** an F-28 und mittelbar M-66 bis M-68, mit **Lückenanteil** (F-19 ohne Vorzeichenregel; M-75 prüft ± ohne Richtung zu benennen) | Vorzeichen von v in F-19 wörtlich festlegen und in M-75 spiegeln; R_X über den tatsächlich gehörten Ausschnitt [a, a+T) statt über [0, p); Ausschnittdatei und SHA-256 an denselben Ausschnitt binden, Überlauf als gemessene Szenario-Voraussetzung; F-27 „v verschiebt nur den Messpunkt" ersetzen; Rotbeweis mit v < 0 und später Klammer |
| 3 — Referenz am publizierten Messstand (F-28 Zeile 413, M-67, M-68) | BESTÄTIGT, in zwei Punkten schärfer: (a) leichte und schwere Felder derselben Antwort tragen verschiedene Materialzeiten, die Vorbedingung wird also an einer frischeren Zeit geprüft als die Bänder; (b) der Probeeq-Worker ruft weder `auswerten()` noch `auswertenLeicht()` — sein `snapshot()` bleibt an HEAD dauerhaft `keineDaten` | **DEFEKT** (a) an F-28/M-67/M-68 (Bänder ohne hergeleiteten Bezugspunkt, P-5) und **DEFEKT** (b) an F-5/M-67/M-68 (der Beweisweg trägt die Zusage nicht; für §3 Etappe 2 zugleich eine Pfadlücke) | Referenz an der Materialzeit ankern (`gesamt_sekunden` für die leichten, ein additives Materialzeitfeld der schweren Auswertung für die Bandfelder), Positionsklammer nur noch als Plausibilität und roh; ersatzweise Klammerbeginn minus hergeleiteter Latenzschranke, sonst `roh`; Publikation im Probeeq-Worker über dieselbe `WorkerKadenz` bauen, F-5 nachziehen, neue Matrixzeile (Etappe 2, B30) mit Rotbeweis `noch_keine_messdaten` |
| Gemeinsame Ursache | BESTÄTIGT: „Referenz und Messung nicht auf derselben Materialzeit" — vier Glieder (Rasterphase, Kopf des Umlaufs, Versatz v, Publikationslatenz), Fortsetzung von §10.3 Ursache (a) auf der Referenzseite | Maßstab für die Nacharbeit, kein eigener Befund | Jede Zahl in F-28, die ein Band oder eine Vorbedingung trägt, nennt ihre Materialzeit und deren Quelle; ist die Materialzeit nicht aus dem Umschlag lesbar, bleibt der Wert `roh` (P-5) |
