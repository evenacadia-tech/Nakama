# NAK-311 — Quellvalidierung Teil A (lesender Opus-Agent, HEAD 7463c3eb)

**Ticket:** NAK-311 · Planschritt S25j · Etappe 0 „Quellvalidierung", **Teil A**
**HEAD:** `7463c3ebe78c2637bec9da42e23d7f6f2bd9af38` (Zweig `master`)
**Datum:** 19.09.2026 · **Modell:** Opus (lesend)
**IDs (5):** T3-15-05 (W01), T3-14-02 und T3-15-08 (W03), T3-01-01 und T3-09-02 (W04)
**Quelle der Befunde:** `docs/audits/2026-09-15-tiefenaudit/BEFUNDE.md` (Phasenzeilen,
Skeptikerzeilen Phase 10, Phase-16-Nachtrag, Arbeitsübersicht W01/W03/W04) und die
Rohberichte unter `docs/audits/2026-09-15-tiefenaudit/roh/`, insbesondere
`phase-16-d-dsp.md` und `phase-16-f-gegenpruefung-d.md`.
**Gate:** Schritt S25j in `docs/plan/plan.json`; Registerzeile `docs/offene-punkte.md:316`.
**Muster:** `docs/beweise/roh/NAK-309-quellvalidierung-teil-A.md`.

**Prüfform:** ausschließlich Quellenlektüre am HEAD. Kein Bau, kein Testlauf, kein Kanon,
kein FL Studio, kein Codex, keine Fehlerinjektion. Git nur lesend. Jede Zeilennummer unten
ist am HEAD `7463c3eb` selbst nachgelesen, keine aus dem Audit übernommen.
Sicherheitsbefunde (T3-12-\*) sind nicht Gegenstand; Legacy-Bezeichner (`EQ-Copilot`,
`EqCop*`, `Eqcp`) sind kein Befund.

**Arbeitsbaum, geprüft mit `git status --short`.** *Zu Beginn des Laufs* und *beim Schreiben
dieser Datei* identisch: nur die zwei fremden untracked Ordner `briefing-hub/` und
`nimbalyst-local/`, sonst nichts; HEAD unverändert `7463c3eb`. Der HEAD ist während des
Laufs nicht gewandert; ein Basisabgleich war deshalb nicht nötig. Diese Datei ist die
einzige Schreibaktion des Agenten.

**Basis der Auditmessungen gegen HEAD.** Die Phase-16-Pakete D und F maßen am Stand
`aff2d8188f33a9525aec1869449773bfdbe305a6`. `git diff --stat aff2d818 7463c3eb -- eq-copilot
broker tools` berührt **keine** der hier zitierten Dateien: nicht `eq-copilot/plugin/dsp/`,
nicht `eq-copilot/plugin/sonde/`, nicht `eq-copilot/plugin/state/`, nicht
`eq-copilot/plugin/tests/DspGoldenTestMain.cpp`, nicht
`eq-copilot/plugin/tests/SondeNullTestMain.cpp`, nicht `tools/fl/nulltest.py`. Geändert
wurden dort nur Pipe-, Installer-, Runner- und Laufzeitteile (`broker/src/briefkasten.rs`,
`eq-copilot/plugin/src/*`, `tools/beweise.ps1`, `tools/fl/laufzeit.ps1`,
`tools/fl/szenario.py` und weitere). Die Auditmessungen gelten damit unverändert für den
HEAD; verschoben haben sich nur einzelne Zeilennummern in `tools/beweise.ps1`, die unten
neu gelesen sind.

**Ergebnis in einer Zeile je ID**

| ID | Urteil | Kategorievorschlag | Schwere | Rotbeweis fällt an |
|---|---|---|---|---|
| T3-15-05 | BESTÄTIGT (Zahl unabhängig aus der Quelle nachgerechnet, exakt getroffen) | DEFEKT | high | Bein B6 `DspGoldenTestMain.cpp`, neuer Fall neben `:1115` (Abschnitt B, M-07); Behauptungszeile `tools/beweise.ps1:723` |
| T3-14-02 | BESTÄTIGT | DEFEKT (M-121 ist gebrochen) | medium | Bein B6 `DspGoldenTestMain.cpp:4047-4058` (M-121-Block), zweiter Unterfall ohne Audio dazwischen; `tools/beweise.ps1:723` |
| T3-15-08 | BESTÄTIGT | LÜCKE (E-8 deckt den Fall nicht, Produktentscheid nötig) | medium | Bein B6 `DspGoldenTestMain.cpp`, neuer Fall im E-19-/Rampenabschnitt; `tools/beweise.ps1:723` |
| T3-01-01 | PRÄZISIERT (Quellkette trägt; ein Ausführungspunkt offen; „ausgeschaltet" ist **nicht** betroffen) | DEFEKT gegen M-02, dazu Korpuslücke | medium (Audit: critical) | Bein A16 `SondeNullTestMain.cpp:356-373` mit erweitertem Material aus `:77-83`; `tools/beweise.ps1:688` |
| T3-09-02 | BESTÄTIGT (Fehler des Beweiswerkzeugs, kein beobachteter Produktbruch) | DEFEKT (Beweiswerkzeug) | medium | Bein A35 `tools/fl/nulltest.py:615-637` (`fall_identisch_null_abweichungen`) gegen `:206` / `:226`; `tools/beweise.ps1:642` |

---

## 1. T3-15-05 — die fünf Pfadrampen überdauern das Ausschalten (W01)

*Befundzeile: `BEFUNDE.md:631` · Gegenprüfung Phase 15: `:660` · Phase-16-Nachtrag:
`:731` · Arbeitsübersicht W01: `:804` · Rohbelege: `roh/phase-16-d-dsp.md:67-80`,
`roh/phase-16-f-gegenpruefung-d.md:64-87`*

### 1.1 Quellkette, zitiert

**Station 1 — was eine Pfadrampe ist und wo sie lebt.** `eq-copilot/plugin/dsp/DspKern.h:310-323`:

```
    struct Rampe
    {
        double aktuell { 1.0 }, ziel { 1.0 }, schritt { 0.0 };
        int    rest { 0 };

        void setzeSofort (double z) noexcept { aktuell = ziel = z; schritt = 0.0; rest = 0; }

        void setzeZiel (double z) noexcept
        {
            if (z == ziel) return;
            ziel    = z;
            schritt = (z - aktuell) / (double) kRampeSamples;
            rest    = kRampeSamples;
        }
```

`:339-344` nennt die fünf: `Rampe input, output, mix, width, autoGain;`. Sie gehören dem
**Pfad** (`PfadZustand`, `:355-359`), nicht der Bank.

**Station 2 — der einzige Ort, der sie auf 1,0 stellt.**
`eq-copilot/plugin/dsp/DspKern.cpp:85-93` (`PfadZustand::ruhe()`) enthält
`rampen.setzeSofort (1.0);` in `:91`. Aufgerufen wird `ruhe()` genau zweimal: `:110` in
`bereiteVor` und `:138` in `freigeben`. **Im laufenden Betrieb wird keine Pfadrampe je auf
1,0 zurückgesetzt.**

**Station 3 — das Ausschalten lässt die Ziele stehen.** `DspKern.cpp:374-394`, die
ENDE-Marke im Blockrand:

```
    if (neu == DspBankPool::kEnde)
    {
        // B-8 / B-10: in die Ruhe, ueber denselben Crossfade wie jeder
        // topologische Wechsel. Ist der Pfad schon in Ruhe, gibt es nichts
        // auszublenden.
        if (alt < 0) return;
        baenke.beginneVerblassen (alt);
        z.quelle = alt;
        z.aktiv  = -1;
        // Die Rampen behalten ihre Ziele: die ausblendende Bank klingt mit
        // IHREN Gains aus, genau wie beim Hard-Bypass-Wechsel (M-06).
```

Zwei Ausgänge, beide ohne Rampenpflege: der Absichtskommentar `:383-384` hält das für den
ausblendenden Fall ausdrücklich fest, und `:379` kehrt zurück, **bevor** überhaupt etwas
geschieht, wenn der Pfad schon in Ruhe ist. Genau dieser zweite Ausgang trifft jede weitere
Publikation bei ausgeschaltetem EQ — also auch die, die `output_trim_db` von +24 dB auf
0 dB stellt.

**Station 4 — das Einschalten rampt vom alten Iststand.** `DspKern.cpp:416-424`:

```
    // B-6: die Rampen DIESES Pfades laufen auf die Ziele DIESES Programms.
    const auto& pn = bankNeu.programm;
    z.rampen.input.setzeZiel (pn.inputTrimLin);
    z.rampen.output.setzeZiel (pn.outputTrimLin);
    z.rampen.mix.setzeZiel (pn.mix);
    z.rampen.width.setzeZiel (pn.width);
```

`setzeZiel` (Station 1) rechnet `schritt` aus `aktuell`, und `aktuell` steht noch auf dem
linearen Wert von +24 dB. Gleichzeitig ist der Übergang aus der Ruhe heraus ein Crossfade
(`:398` `nurRampen` ist bei `alt < 0` false, `:413-414`), dessen Quelle das **unveränderte
Dry** ist (`:762-768`).

**Station 5 — Anwendung und Mischung.** `DspKern.cpp:713-732` wendet die Rampe an:

```
        if (! outRuht) { l *= out; rr *= out; }
```

`DspKern.cpp:796-805` mischt linear:

```
        if (crossfade && rest > 0)
        {
            const double t = 1.0 - (double) rest / (double) kFadeSamples;
            ausL[i] = vonL[i] * (1.0 - t) + nachL[i] * t;
```

`eq-copilot/plugin/dsp/DspProgramm.h:53` und `:57`: `kFadeSamples = 256`,
`kRampeSamples = 256`.

**Station 6 — `reset()` heilt nicht, es friert ein.** `DspKern.cpp:182-185`:

```
        // Die Rampen stehen auf ihrem Ziel - das Ziel ist das Programm, der
        // Weg dorthin ist Historie.
        for (auto* r : { &z.rampen.input, &z.rampen.output, &z.rampen.mix, &z.rampen.width, &z.rampen.autoGain })
            r->setzeSofort (r->ziel);
```

`ziel` ist hier das **alte** Ziel (+24 dB), nicht das des aktuellen Programms.
`eq-copilot/plugin/sonde/SondeProcessor.cpp:287-309` ruft genau das aus dem
JUCE-`reset()`; sein Absichtskommentar `:299-300` sagt dabei „Zurueck geht nur die
Audiohistorie des Kerns: Filterzustaende, **Rampen**, Crossfades." — die Rampen gehen
gerade **nicht** zurück, sie werden auf dem veralteten Ziel festgesetzt. Das ist ein
irreführender Kommentar am selben Befund.

**Erreichbarkeit: Stufe 1 (gewöhnliche FL-Automation).**
`eq-copilot/plugin/state/NakamaParameter.cpp:51` führt
`gleitkomma ("v1.global.output_trim_db", -24.0, 24.0, 0.0)`, `:79`
`boolean ("v2.global.eq_enabled", false, false, true, 2)`; beide liegen unter den 112
Hostparametern (`eq-copilot/plugin/state/NakamaParameter.h:44`, `:51`). Der Weg ist
geschlossen: `SondeProcessor.cpp:1296-1304` (`parameterValueChanged`, atomare Mailbox)
→ `:1421-1432` (`dspKontrollTakt` → `automationSchreiben`) → `:1442-1443`
(`publiziereWirksam`). Kein Gen, kein belegtes Band, kein UI-Handgriff nötig.

### 1.2 Die Zusage, wörtlich

- `CLAUDE.md:29` (Grundgesetz): „**Grundgesetz: nichts Ungefragtes.** Verarbeitung nur, wenn
  der User sie einschaltet."
- SONDE-015 M-03, `docs/beweise/SONDE-015.md:1036`: „Der Wechsel ist **klickfrei** und
  topologisch: die Übernahme geschieht am Blockrand, davor liegt ein Crossfade fester Länge
  (`kFadeSamples`, §5.2)."
- Das Maß dazu, Entscheid E-31, `docs/beweise/SONDE-015.md:3425`: „Am Umschaltsample: die
  Nachbarsample-Differenz am ersten Sample des neuen Fades ist höchstens die Fadeschrittweite
  plus die Rundungstoleranz 2^−23 = 1,19e-7. Die Maximumsprüfung über den ganzen Lauf bleibt
  als Wache …"
- Entwurf `docs/FL-Nakama-Sonden-Design-Entwurf.md:2990-2991`: „Der Audiothread liest ein
  unveränderliches `DspProgram`, allokiert und sperrt nie. Stetige Parameter werden geglättet;
  Bandtyp, Kanalmodus oder Bandanzahl wechseln per kurzem Crossfade zwischen zwei vollständig
  vorbereiteten Programmen."
- **Keine** Zusage trägt der Befund über M-07 (`docs/beweise/SONDE-015.md:1040`): M-07 redet
  von Filter- und Hüllkurvenzuständen, nicht von den Pfadrampen. Die Befundzeile sagt das
  bereits selbst; die Lektüre bestätigt es.

### 1.3 Heutige Abdeckung und warum sie den Befund nicht fängt

| Prüfstelle | Was sie misst | Warum sie den Befund nicht fängt |
|---|---|---|
| B6 `DspGoldenTestMain.cpp:1089-1117`, Fall `ausgeschaltet_rechnet_nichts_und_startet_kalt` (M-07) | schaltet mit `machSatz (false)` aus (`:1103`), wartet `kFadeSamples + 1024` (`:1104`), schaltet **dasselbe** Programm wieder ein (`:1109`) und misst bei **Stille** am Eingang `maxAus == 0.0` (`:1113-1116`) | Kein Trimwechsel im ausgeschalteten Zustand, und Stille am Eingang: 0 mal jeder Rampenwert ist 0. Der Fall ist gegen den Befund blind. |
| B6 `DspGoldenTestMain.cpp:778`, `engagieren_ist_klickfrei_und_endet_deterministisch` (M-03) | das Einschalten aus dem Ruhezustand | Dort stand nie ein von 1,0 verschiedener Rampenwert im ausgeschalteten Zustand. |
| B6 `DspGoldenTestMain.cpp:1071-1083`, E-31-Wache (`groessterSprung` `:471`, `kaltSchranke` `:498`) | Sprung am Umschaltsample und Maximumwache über den Lauf | Misst einen topologischen Wechsel zwischen zwei **eingeschalteten** Programmen, nie den Weg aus/ein mit dazwischen geändertem Globalwert. |
| A16 `SondeNullTestMain.cpp:356-373` | Bitidentität bei neutral eingeschaltetem Kern | Neutral heißt hier Trim 0 dB von Anfang an; nie ein zurückgenommener Gain. |

Zwischen den beiden gemessenen Nachbarn liegt genau der ungeprüfte Fall: **Global ändern,
während der Pfad in Ruhe ist.**

### 1.4 Urteil: BESTÄTIGT, Zahl unabhängig aus der Quelle nachgerechnet

Ich habe die Zahl **nicht** übernommen, sondern aus den Quellstellen 1, 3, 4, 5 neu
gerechnet. Parameter des Rohbelegs (`roh/phase-16-f-gegenpruefung-d.md:66`): 48 kHz, DC 0,5,
EQ an mit Output-Trim +24 dB und 1024 Samples Einschwingen, aus und 512 Samples, Trim bei
ausgeschaltetem EQ auf 0 dB und 512 Samples, wieder an.

Beim Wiedereinschalten steht `aktuell = 10^(24/20) = 15,848931924611133`, `ziel` wird 1,0,
`schritt = (1,0 − 15,848931924611133)/256 = −0,058003640330512`. Weil `tick()` erst addiert
und dann liefert (`DspKern.h:325-334`), gilt für das nullbasierte Sample *i*

`out(i) = 15,848931924611133 − (i+1)·0,058003640330512`

und für das Crossfade-Gewicht `t(i) = i/256` (aus `rest = kFadeSamples` und
`t = 1 − rest/kFadeSamples`, `DspKern.cpp:793-804`). Die Quelle des Fades ist Dry, also der
Eingang selbst; das Verhältnis Ausgang/Eingang ist deshalb

`v(i) = (1 − i/256) + (i/256)·out(i)`

- `v(127) = 0,50390625 + 0,49609375·8,424465962 = 4,683231161`
- `v(128) = 0,5 + 0,5·8,366462322 = 4,683231161`
- `20·log10(4,683231161) = 13,410912 dB`
- größter Nachbarsprung `v(1) − v(0) = 1,057550430 − 1,0 = 0,057550430` (bei Eingang 0,5:
  **0,028775215**)

Gemessen im Rohbeleg (`roh/phase-16-f-gegenpruefung-d.md:70-77`): `maxRatio=4.68323135375977`,
`maxDb=13.4109122531407`, `maxAt=127`, `jump=0.0287752151489258`, `finalRatio=1`. **Jede
Stelle stimmt** (die Reste sind die float-Verengung des Ausgangs). Die Zahl **+13,41 dB ist
damit am HEAD aus dem Quelltext allein reproduzierbar.**

Drei Präzisierungen:

1. Das Maximum ist ein **Plateau über zwei Samples** (nullbasiert 127 und 128), nicht ein
   einzelner Ausschlag; `maxAt=127` nennt nur das erste. Der Rohbericht `phase-16-d-dsp.md:78`
   hat die Indexbasis bereits richtiggestellt; die Zweisamplebreite ergänze ich.
2. `+13,41 dB` gilt **für den Output-Trim**. Der Mechanismus gilt für alle fünf Rampen
   (gemeinsame Struktur `DspKern.h:339-344`, gemeinsame Zielsetzung `DspKern.cpp:418-424`),
   die **Größe** aber nicht: `mix` und `width` sind durch ihre eigenen Wertebereiche
   gedeckelt, `input` trägt dieselben ±24 dB wie `output`, `autoGain` ist heute ungeklemmt
   (das ist T3-15-09 in W08). Die Befundzeile sagt „Gilt ebenso für Input-Trim, Mix, Width und
   Auto-Gain" — richtig als Mechanismus, nicht als Zahl. `roh/phase-16-f-gegenpruefung-d.md:87`
   mahnt genau das an.
3. Die Gegenrichtung (−24 dB → 0 dB) erzeugt laut Rohbeleg **keinen** Peak über 1; auch das
   folgt aus der Formel: `out(i)` startet dann unter 1 und das Dry-Gewicht hält `v(i)` unter 1.

### 1.5 Kategorie und Schwere

**DEFEKT, high.** Es bricht eine geschriebene Zusage (M-03 klickfrei mit dem Maß E-31) und
das Grundgesetz (ein Gain, den der User zurückgenommen hat, wird trotzdem ausgeführt). Der
Befund ist heute ohne Gen, ohne belegtes Band und ohne Spezialbedienung über zwei gewöhnliche
Hostparameter erreichbar, und +13,4 dB auf einem Bus sind hörbar. Ich teile die Hochstufung
der Phase-15-Gegenprüfung (`BEFUNDE.md:660`).

### 1.6 Kleinster Fix und Wechselwirkungen

**Ort:** `DspKern::blockrand`, `eq-copilot/plugin/dsp/DspKern.cpp:357-425`.

**Idee.** Der Übergang **aus der Ruhe** hat kein altes Programm, von dem weggerampt werden
müsste: liegt `alt < 0`, müssen die fünf Rampen mit `setzeSofort` auf die Ziele des neuen
Programms gesetzt werden statt mit `setzeZiel` von einem fremden Iststand aus zu starten. Die
ENDE-Marke muss zusätzlich auch im Fall `alt < 0` (`:379`) die fünf Ziele auf das jetzt
gültige — ausgeschaltete, also neutrale — Programm nachziehen, damit ein späteres
`beendeAudiohistorie()` (`:182-185`) nicht auf einem veralteten `ziel` einfriert. Beides sind
fünf Skalarzuweisungen am Blockrand: keine Allokation, keine Sperre, kein Zweig im
Samplepfad. Der ausblendende Pfad behält seinen Gain (`:383-384` bleibt für `alt >= 0`
gültig), weil der Quelllauf eines Crossfades seinen Rampenstand ohnehin gesichert und
zurückgesetzt bekommt (`:756-760`).

**Wechselwirkungen.** Keine gemeinsame Ursache mit W03: dort geht es um den **Bank**-Zustand
(Filter, Detektor, Hüllkurve je Slot), hier um den **Pfad**-Zustand (fünf Skalare je Pfad).
Sehr wohl gemeinsam ist die *Wurzel* beider: `blockrand` entscheidet allein aus den zwei
Programmen, die es gerade sieht, und kennt weder die Generation noch die Belegungsidentität
dessen, was dazwischen verdrängt wurde oder in der Ruhe geschah. Die Arbeitsübersicht nennt
das bei W01 „generationstreu zum aktuellen Programm" (`BEFUNDE.md:804`). W04 ist unabhängig.
`BEFUNDE.md:806` verlangt ausdrücklich, W03 **nicht** über W01 oder einen vollständigen Reset
zu lösen — der Befund hier bestätigt das: ein Reset (`:182-185`) friert den Fehler ein,
statt ihn zu heilen.

### 1.7 Ort des Rotbeweises

**Bein B6** (`EqCopDspGoldenTest`, Behauptungszeile `tools/beweise.ps1:723`), Datei
`eq-copilot/plugin/tests/DspGoldenTestMain.cpp`, neuer Fall unmittelbar neben dem
M-07-Fall `:1089-1117` (Abschnitt B, dort steht die vorhandene Aus/Ein-Mechanik).

Aufbau: 48 kHz, DC 0,5, EQ an mit Output-Trim +24 dB, einschwingen; aus, ausblenden;
Output-Trim 0 dB bei ausgeschaltetem EQ; wieder an; über **alle** Samples des
Wiedereinschaltens messen. Assertionen:

- `peak <= 1.0 + 1e-6` über den ganzen Lauf — **heute 4,6832**, also rot;
- Endwert exakt 1,0 (hält heute schon, sichert den Fix gegen Überschießen nach unten);
- Nachbarsprungwache über `groessterSprung (…)` (`:471`) gegen die E-31-Schranke — heute
  0,0575 je Sample;
- Gegenfall ohne Trimwechsel (0 dB → aus → 0 dB → an): exakt 1,0, Sprung 0 — hält heute und
  muss halten bleiben;
- Gegenfall mit `beendeAudiohistorie()` im ausgeschalteten Zustand: derselbe Peak — sichert,
  dass der Fix nicht nur den Resetweg repariert;
- Wiederholung über die Blockteilungen 1, 64, 333 und gemischt (Paket D belegt
  Blockunabhängigkeit, `roh/phase-16-d-dsp.md:73`).

Die Behauptungszeile `tools/beweise.ps1:723` muss den Fall ausdrücklich nennen, sonst misst
das Bein etwas, das das Manifest nicht behauptet.

---

## 2. T3-14-02 — neue Belegung erbt die Dynamik des Vorgängers (W03, Richtung 1)

*Befundzeile: `BEFUNDE.md:589` · Phase-16-Nachtrag: `:737` · Arbeitsübersicht W03: `:806` ·
Rohbelege: `roh/phase-16-d-dsp.md:82-90`, `roh/phase-16-f-gegenpruefung-d.md:89-104`*

### 2.1 Quellkette, zitiert

**Station 1 — Remove und Belegen sind zwei gewöhnliche, erfolgreiche Transaktionen.**
`eq-copilot/plugin/state/NakamaTransaktion.cpp:536-551`:

```
                case Art::remove:
                {
                    if (! committed.werte[(size_t) param::indexOccupied (a.slot)].b) { grund = "slot_frei"; break; }
                    // R5: occupied = false UND alle dreizehn v1-Werte auf Default;
                    // kein anderer Slot wird beruehrt oder umnummeriert.
```

und `:525-535` für `Art::bandBelegen`, das über `ersterFreierSlot()` genau denselben Slot
wieder nimmt und `indexOccupied (slot)` auf `true` setzt.

**Station 2 — die zweite Publikation verdrängt die erste, bevor Audio sie sieht.**
`eq-copilot/plugin/dsp/DspBankPool.cpp:137-151`:

```
    const std::uint64_t vorher = veroeffentlicht[(size_t) p].exchange (
        publikationsWort (g, slot), std::memory_order_acq_rel);
    return verdraengeBereiten (vorher);
```

`verdraengeBereiten` (`:123-135`) gibt die noch nicht übernommene Bank frei. Läuft zwischen
Remove und Neubelegung kein Audioblock, sieht der Audiothread das Remove **nie**.

**Station 3 — die Vergleichsregel kennt keine Belegungsidentität.**
`eq-copilot/plugin/dsp/DspProgramm.cpp:139-159`, `rampenKompatibel`, vergleicht je Slot genau
dies:

```
        if (a.aktiv != n.aktiv) return false;
        if (! a.aktiv) continue;
        if (a.typ != n.typ || a.modus != n.modus || a.dynamisch != n.dynamisch
            || a.nutztSvf != n.nutztSvf || a.quelle != n.quelle)
            return false;
```

Die alte aktive Bank trug Slot 0 aktiv mit genau diesen Merkmalen; das endgültig publizierte
Programm ebenso. Damit ist der Wechsel „nur Rampen".

**Station 4 — und dann wandert der Zustand.** `eq-copilot/plugin/dsp/DspKern.cpp:398-408`:

```
    const bool nurRampen = alt >= 0 && rampenKompatibel (baenke.bank (alt).programm, bankNeu.programm);

    if (nurRampen)
    {
        // B-4: der Filter- und Huellkurvenzustand WANDERT mit. …
        bankNeu.baender         = baenke.bank (alt).baender;
        bankNeu.monoBassZustand = baenke.bank (alt).monoBassZustand;
    }
```

**Station 5 — was genau mitwandert.** `eq-copilot/plugin/dsp/DspBankPool.h:58-72`:
`BandZustand` trägt `BiquadZustand statisch[2]`, `SvfZustand svf[2]`,
`BiquadZustand detektor[2]`, `HuellkurveZustand huelle`, `schrittRest`, `svfVon`/`svfNach`
und `double auslenkungDb`. `eq-copilot/plugin/dsp/DspFilter.h:295` zeigt, was die
Hüllkurve hält: `void nullen() noexcept { leistung = 0.0; holdRest = 0; }` — also Leistung
**und** Hold. Genau diese Werte erbt das frisch belegte Band.

**Erreichbarkeit.** Zwei erfolgreiche Transaktionen ohne Audioblock dazwischen; Paket D hat
das über den echten `Transaktionskern` mit `DspKernAusfuehrung` gefahren und beide Commits
asserted (`roh/phase-16-d-dsp.md:84`, `roh/phase-16-f-gegenpruefung-d.md:91-97`:
`removeRevision=1 addRevision=2 slot=0`, `dynamicDb=-12 audioPeak=0.102507740259171` aus
**Stille**). Die Bedienoberfläche zum Belegen ist noch nicht angeschlossen (NAK-306), der
gebaute API-Vertrag aber schon.

### 2.2 Die Zusage, wörtlich

SONDE-015 M-121, `docs/beweise/SONDE-015.md:1213`: „Wechselt Programm, Belegung oder
Slotinhalt — Remove, Neubelegung, Typwechsel, `dynamic_enabled` aus —, wird der gehaltene
Wert dieses Slots **verworfen** (0) statt weitergemeldet." In derselben Zeile steht der
Rotbeweis ausdrücklich als: „oder ein neu belegter Slot erbt den Wert seines Vorgängers".
Dazu die Regel B-06, `docs/beweise/SONDE-015.md:2236`.

Die Zusage deckt beide gemessenen Hälften: `dynamicDb=-12` **ist** der gehaltene, gemeldete
Wert (`auslenkungDb`, gelesen über `DspKern::auslenkungenDb`, `DspKern.cpp:310-314`), und der
Audio-Peak 0,1025 ist die Folge derselben geerbten Hüllkurve. Für die Audiohälfte allein gibt
es **keine eigene Zusage**; sie ist über M-121 aber mitgetroffen.

### 2.3 Heutige Abdeckung und warum sie den Befund nicht fängt

Bein B6, Fälle `remove_verwirft_die_alte_auslenkung` und
`remove_mit_neubelegung_verwirft_die_alte_auslenkung`,
`eq-copilot/plugin/tests/DspGoldenTestMain.cpp:4010-4059`. Zwei Abweichungen von der
Befundfolge, jede für sich ausreichend, den Fall zu verfehlen:

1. **Audio läuft dazwischen.** `:4038` und `:4052` fahren jeweils
   `fahreStille (*k, kFadeSamples + 2048, 512);` zwischen den beiden
   `uebernehmeZustand`-Aufrufen. Damit nimmt der Audiothread das Remove; die verdrängende
   Publikation aus Station 2 entsteht gar nicht.
2. **Die Neubelegung ist topologisch anders.** `:4050`
   `belege (neu, 2, Filtertyp::notch, 3000.0, 4.0, 0.0);   // statisch` — ein statisches Band
   gegen ein vorher dynamisches. `dynamisch` und `nutztSvf` unterscheiden sich, also liefert
   `rampenKompatibel` (Station 3) false und die Bank startet ohnehin kalt.

Der Test ist deshalb grün **und** blind für den Befund. Das ist keine Nachlässigkeit, sondern
genau die Gegenfolge, die `BEFUNDE.md:589` benennt.

### 2.4 Urteil: BESTÄTIGT

Die Quellkette ist am HEAD vollständig und schließt ohne Lücke: erfolgreicher Commit →
verdrängte Zwischenpublikation → `rampenKompatibel` sieht keinen Belegungswechsel →
Zustandskopie einschließlich Detektor, Hüllkurve, Hold und `auslenkungDb`. Die
Auditmessungen (Revision 1/2, −12 dB, Peak 0,1025 aus Stille, frische Kontrolle exakt 0)
konnte ich auftragsgemäß nicht nachfahren; sie sind in `roh/phase-16-d-dsp.md:84` und
`roh/phase-16-f-gegenpruefung-d.md:91-98` als ausgeführte Komponentenläufe belegt und mit der
Quellkette konsistent.

### 2.5 Kategorie und Schwere

**DEFEKT, medium.** M-121 ist eine geschriebene Zusage mit genau diesem Wortlaut; sie ist
gebrochen. Gegen „high" spricht die Erreichbarkeit: das Belegen ist heute nur über
Transaktion oder geladenen Zustand erreichbar, nicht über eine vorhandene Bedienfläche
(NAK-306). Die Arbeitsübersicht `BEFUNDE.md:806` nennt das „Belegen Stufe 2".

### 2.6 Kleinster Fix und Wechselwirkungen

**Ort:** `DspKern::publiziereVorbau` (`eq-copilot/plugin/dsp/DspKern.cpp:237-276`) und die
Übernahme in `DspKern::blockrand` (`:397-408`).

**Idee.** Der Audiothread kann die verdrängte Zwischenpublikation nicht sehen — der Worker
sehr wohl. `publiziereVorbau` merkt sich je Pfad das zuletzt publizierte Programm und legt
auf der neuen Bank ein Flag ab: `bank.kompatibelZuVorgaenger = rampenKompatibel (zuletzt
publiziertes Programm, prog)`. `blockrand` verlangt dann **beides** — die heutige
Live-Prüfung gegen die aktive Bank **und** dieses Flag. Damit bricht jede verdrängte
Zwischenpublikation die Kompatibilitätskette, ohne dass der Audiothread etwas Zusätzliches
rechnet: ein `bool`-Lesen am Blockrand, keine Allokation, kein Vertrags- oder
Stateformatwechsel (was `BEFUNDE.md:806` ausdrücklich ausschließt).

**Wechselwirkungen.** T3-14-02 und T3-15-08 sind **dieselbe fehlende Sache in zwei
Richtungen**: es gibt keine Slot-Lebenszykluskennung. Eine pauschale Reparatur „immer alles
nullen" behebt 14-02 und verschärft 15-08; „immer alles kopieren" umgekehrt — beides steht
so in `roh/phase-16-d-dsp.md:90` und `roh/phase-16-f-gegenpruefung-d.md:104` und hält der
Quellenlektüre stand. Beide gehören in **einen** Änderungssatz und in **eine** Regression.
Keine Verbindung zu W01 (Pfadrampen) oder W04.

### 2.7 Ort des Rotbeweises

**Bein B6** (`tools/beweise.ps1:723`), `eq-copilot/plugin/tests/DspGoldenTestMain.cpp`,
M-121-Block `:4010-4059`: ein **dritter** Unterfall (c) direkt hinter `:4058`.

Aufbau: dynamisches Bell 1 kHz, Q 2, Range −12 dB, Threshold −40 dB, Attack 0,1 ms,
Hold 500 ms, Release 5000 ms in Slot 0 warmfahren; dann `uebernehmeZustand (remove)` und
`uebernehmeZustand (belegen)` **ohne** `fahreStille` dazwischen, mit **identischer**
Bandkonfiguration; danach nur Stille. Assertionen:

- `auslenkungenDb()[0] == 0.0` — heute **−12**, also rot;
- Peak des Ausgangs über die ersten 512 Stillesamples `== 0.0` — heute **0,1025**;
- Gegenfall frische Kontrollinstanz mit derselben Belegung: 0 und Stille (hält heute);
- Gegenfall gewöhnliche stetige Wertänderung desselben Slots: Historie **bleibt** erhalten
  (E-19, `docs/beweise/SONDE-015.md:3412`) — schützt gegen die Pauschalreparatur.

---

## 3. T3-15-08 — ein Fremdslotwechsel startet alle Bänder kalt (W03, Richtung 2)

*Befundzeile: `BEFUNDE.md:634` · Phase-16-Nachtrag: `:737` · Arbeitsübersicht W03: `:806` ·
Rohbelege: `roh/phase-16-d-dsp.md:88`, `roh/phase-16-f-gegenpruefung-d.md:100-102`*

### 3.1 Quellkette, zitiert

**Station 1 — jede Aktivänderung irgendeines Slots macht den ganzen Wechsel topologisch.**
`eq-copilot/plugin/dsp/DspProgramm.cpp:148-157` (dieselbe Schleife wie in Abschnitt 2,
hier in der Gegenrichtung gelesen):

```
    for (int i = 0; i < kSlots; ++i)
    {
        const auto& a = alt.baender[(size_t) i];
        const auto& n = neu.baender[(size_t) i];
        if (a.aktiv != n.aktiv) return false;
```

`return false` ist global: ein einziger Slot genügt, und **alle acht** verlieren die
Kompatibilität.

**Station 2 — die neue Bank wird kalt publiziert.**
`eq-copilot/plugin/dsp/DspKern.cpp:262-271`:

```
    // M-07/E-8: die neue Bank startet KALT. Ist der Wechsel am Blockrand ein
    // reiner Rampenwechsel, uebernimmt der Audiothread dort den Zustand der
    // laufenden Bank (B-4) …
    bank.zustaendeNullen();
    for (int i = 0; i < kSlots; ++i)
    {
        auto& z = bank.baender[(size_t) i];
        z.svfVon = z.svfNach = bank.programm.baender[(size_t) i].svfRuhe;
    }
```

`zustaendeNullen` ist `eq-copilot/plugin/dsp/DspBankPool.cpp:38-42`:

```
void DspBank::zustaendeNullen() noexcept
{
    for (auto& b : baenke… ) b.nullen();
    monoBassZustand.nullen();
}
```

(wörtlich `for (auto& b : baender) b.nullen();`), und `b.nullen()` trifft nach
`DspBankPool.h:58-72` statische Biquads, SVF, **Detektor**, **Hüllkurve** und `auslenkungDb`.

**Station 3 — der Zustandstransfer bleibt aus.** `DspKern.cpp:398-408`: weil Station 1 false
liefert, ist `nurRampen` false, der `if`-Block wird übersprungen, und der Übergang ist ein
Crossfade über `kFadeSamples` (`:413-414`, Mischung `:796-805`).

**Länge des Fades gegen die Länge des Einschwingens.** `DspProgramm.h:53`:
`kFadeSamples = 256` — bei 48 kHz **5,33 ms**. Die Auditmessung nennt für ein unverändertes
Bell 50 Hz / Q 8 / +12 dB **−8,002865 dB** im ersten Perioden-RMS und eine Rückkehr innerhalb
1 dB erst nach **200 ms** (`roh/phase-16-f-gegenpruefung-d.md:100`).

**Erreichbarkeit: Stufe 1 für den Auslöser.** `v1.band.<n>.enabled` an einem belegten Slot
ist ein Hostparameter; die Belegung selbst ist heute Stufe 2 (Transaktion oder Zustand,
NAK-306).

### 3.2 Die Zusage, wörtlich

**Keine Zusage zur Historie unveränderter Slots gefunden.** Im Gegenteil ist der Kaltstart
ausdrücklich entschieden, Entscheid E-8, `docs/beweise/SONDE-015.md:3400`: „Ob eine neue Bank
den Filterzustand der alten erbt | **Nein** — sie startet kalt, alle Zustände auf 0. |
M-07 verlangt genau das … Der Crossfade über 256 Samples deckt die Transiente ab; ein
übertragener Zustand wäre ein geteilter Zustand zwischen zwei Bänken, und §44.2 verbietet das
ausdrücklich."

Beide Begründungen dieses Entscheids halten der Quellenlektüre **nicht** stand:

- „Der Crossfade über 256 Samples deckt die Transiente ab" — 256 Samples sind 5,33 ms bei
  48 kHz, gemessen wurden 200 ms bis zurück innerhalb 1 dB.
- „§44.2 verbietet geteilte Zustände" — der Entwurf sagt an der einschlägigen Stelle
  (`docs/FL-Nakama-Sonden-Design-Entwurf.md:2986-2987`): „vier vorallokierte Programmbänke:
  je ein Double-Buffer für Committed und Candidate, damit beide unabhängig warm bleiben und
  innerhalb ihres Pfads klickfrei die Topologie wechseln können". Getrennt gehalten werden
  die **Pfade**, nicht zwei Bänke desselben Pfades — und E-19
  (`docs/beweise/SONDE-015.md:3412`) überträgt dort schon heute ausdrücklich: „nimmt der
  Audiothread am Blockrand den Filter- und Hüllkurvenzustand der alten Bank in die neue".

Die Befundzeile führt genau diese zwei Gegenargumente; beide sind am HEAD nachgelesen und
tragen.

### 3.3 Heutige Abdeckung und warum sie den Befund nicht fängt

- B6, Fall `ausgeschaltet_rechnet_nichts_und_startet_kalt`
  (`DspGoldenTestMain.cpp:1089-1117`) **verlangt** den Kaltstart und misst bei Stille — er
  würde nach einem Fix in dieser Form unberührt bleiben, denn er betrifft den Slot, der selbst
  geändert wurde.
- B6, M-121-Block (`:4010-4059`) misst die gemeldete Auslenkung des **geänderten** Slots, nie
  die eines unbeteiligten.
- Die E-31-Wache (`:1071-1083`) misst Sprünge am Umschaltsample, nicht den Einbruch über
  200 ms danach; ein langsames Wiedereinschwingen erzeugt gerade **keinen** Sprung.

Es gibt am HEAD keinen Fall, der einen unveränderten, warmen, tieffrequenten Slot über einen
Fremdslotwechsel hinweg misst.

### 3.4 Urteil: BESTÄTIGT als LÜCKE

Die Quellkette ist vollständig und eindeutig; die Wirkung ist unbestritten. Was fehlt, ist
eine Zusage, die sie bricht — E-8 entscheidet den Kaltstart sogar ausdrücklich, nur mit zwei
Begründungen, die den Fall des **unveränderten** Slots nicht tragen. Damit bleibt es eine
Lücke mit Produktentscheid, keine Regelverletzung. Das deckt sich mit der
Phase-16-Gegenprüfung (`roh/phase-16-f-gegenpruefung-d.md:102`).

Präzisierung zu den Zahlen: `−8,002865 dB` (erster Perioden-**RMS**, 200 ms) und die ältere
Angabe `−6,57 dB` / 220 ms (Perioden-**Maxima**, `BEFUNDE.md:634`) sind **zwei verschiedene
Messgrößen desselben Vorgangs**, kein Widerspruch — so steht es auch in
`roh/phase-16-d-dsp.md:88`. Für den Folgebeweis muss die Messgröße festgelegt werden,
sonst ist das Erfolgskriterium nicht prüfbar.

### 3.5 Kategorie und Schwere

**LÜCKE, medium.** Hörbar (bis −8 dB über 200 ms auf einem tieffrequenten, hochgütigen Band),
aber ohne gebrochene Zusage und mit Belegung auf Stufe 2. Der Produktentscheid ist offen und
gehört als Karte nach `docs/plan/fragen.json` — das Gate S25j verlangt das bereits.

### 3.6 Kleinster Fix und Wechselwirkungen

**Ort:** `rampenKompatibel` (`eq-copilot/plugin/dsp/DspProgramm.cpp:139-159`) und die
Übernahme `DspKern::blockrand` (`eq-copilot/plugin/dsp/DspKern.cpp:397-408`).

**Idee.** `rampenKompatibel` liefert statt eines `bool` eine **Slotmaske**: je Slot
„identisch und übertragbar" oder nicht, dazu weiterhin das globale Ja/Nein für Samplerate und
Mono-Bass-Stufe. `blockrand` kopiert dann nur die Bänder, deren Bit gesetzt ist
(`bankNeu.baender[i] = baenke.bank (alt).baender[i];` in einer Schleife statt des heutigen
Vollkopierens in `:406`), und lässt den Rest kalt. Der Übergang bleibt bei einer
Topologieänderung ein Crossfade. Die Kosten sind ein `int` als Maske und bis zu acht flache
Wertekopien am Blockrand — keine Allokation, keine Sperre.

**Wechselwirkungen.** Die Maske ist dieselbe Stelle, an der T3-14-02 die
Belegungsidentität braucht: das Bit eines Slots darf nur gesetzt sein, wenn **weder** die
Topologie **noch** die Belegungskennung gewechselt hat (also unter Einbezug des in 2.6
vorgeschlagenen Vorgängerflags). Eine Reparatur, die nur eine Richtung kennt, verschlechtert
die andere. Keine Verbindung zu W01 oder W04. W07 (T3-15-07, Übergangsspitze großer Sprünge)
hängt an derselben Übernahme und darf die Slotmaske nicht gegen sich haben —
`BEFUNDE.md:806` nennt W03 daher als Vorbedingung für W07.

### 3.7 Ort des Rotbeweises

**Bein B6** (`tools/beweise.ps1:723`), `eq-copilot/plugin/tests/DspGoldenTestMain.cpp`, neuer
Fall im Abschnitt der E-19-/Rampenprüfungen (dort liegen die Zwei-Kern-Vergleiche, die dieser
Fall braucht).

Aufbau: Slot 0 mit Bell 50 Hz / Q 8 / +12 dB an einem 50-Hz-Sinus einschwingen; Slot 5 belegt,
neutral (`gain_db` 0) und **ausgeschaltet**; dann ausschließlich `v1.band.5.enabled` auf true.
Referenz ist ein zweiter, identisch eingeschwungener Kern ohne diesen Wechsel. Assertionen:

- Perioden-RMS der ersten vollständigen 50-Hz-Periode nach dem Wechsel innerhalb **±0,5 dB**
  des Referenzkerns — heute **−8,0 dB**, also rot;
- jede Periode über 2 s innerhalb ±0,5 dB — heute erst nach 200 ms innerhalb 1 dB;
- Gegenfall: Typwechsel an **Slot 0 selbst** darf einschwingen (schützt E-8 für den
  tatsächlich geänderten Slot);
- Gegenfall: Bell 1 kHz / Q 2 bleibt auch heute innerhalb (−0,21 dB laut `BEFUNDE.md:634`) —
  grenzt die Regel auf das ein, was sie treffen soll.

Die Messgröße (Perioden-RMS, nicht Periodenmaximum) gehört in die Behauptungszeile
`tools/beweise.ps1:723`, sonst wiederholt sich die Verwechslung aus 3.4.

---

## 4. T3-01-01 — Subnormals gehen unter DAZ/FTZ im neutral eingeschalteten Kern verloren (W04)

*Befundzeile: `BEFUNDE.md:343` · Skeptikerzeile Phase 10: `:495` · Arbeitsübersicht W04:
`:807`*

### 4.1 Quellkette, zitiert

**Station 1 — der Denormalriegel gilt für den ganzen Block.**
`eq-copilot/plugin/sonde/SondeProcessor.cpp:335-337`:

```
void SondeProcessor::processBlock (juce::AudioBuffer<float>& puffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals keineDenormals;
```

Der eingebundene JUCE-8.0.9-Stand im Bauverzeichnis
(`eq-copilot/build/_deps/juce-src/modules/juce_audio_basics/buffers/juce_FloatVectorOperations.cpp:1544-1556`
— **Bauartefakt, keine Repoquelle**) setzt dafür:

```
ScopedNoDenormals::ScopedNoDenormals() noexcept
{
   #if JUCE_USE_SSE_INTRINSICS
    intptr_t mask = 0x8040;
```

`0x8040` ist FTZ (`0x8000`, Flush-To-Zero für Ergebnisse) **und** DAZ (`0x0040`,
Denormals-Are-Zero für Operanden).

**Station 2 — der neutral eingeschaltete Pfad kehrt nicht früh zurück.**
`eq-copilot/plugin/dsp/DspKern.cpp:898-907`:

```
    const bool nachPass   = zc.aktiv  < 0 || istPassthrough (zc.aktiv);
    …
    const bool committedRuht = nachPass && (zc.uebergang == Uebergang::keiner || vonPass);
```

`istPassthrough` ist `:298-302`: `return ! p.eqEngagiert || p.hardBypass;`. Bei
`eq_enabled = true` und `bypass = false` ist der Pfad also **nicht** Passthrough,
`committedRuht` ist false, und der Rückweg `:963-972` wird **nicht** genommen.

**Station 3 — der Zahlenrücklauf läuft trotzdem.** Hinweg `DspKern.cpp:891-895`:

```
    for (size_t i = 0; i < n; ++i)
    {
        dryL[i] = (double) kanaele[0][i];
```

Rückweg `:1052-1079`:

```
    for (size_t i = 0; i < schreibBis; ++i)
    {
        …
        float fl = (float) l;
        if (! std::isfinite (fl) && std::isfinite (l)) { fl = 0.0f; … }
        kanaele[0][i] = fl;
```

Der Schreibverzicht `:1043-1050` (`schreibBis`) greift nur am **Ende eines
Passthrough-Übergangs**, nicht im stabilen neutralen Zustand.

**Station 4 — im neutralen Zustand ist die Konversion die einzige Rechnung am Sample.**
`verarbeiteBank` kurzschließt alles: Input-Trim `:651-654` (`r.input.ruhtBei (1.0)` → nur
`tick()`), M/S-Stufe `:668-671`, und der Ausgangsblock `:709-732` prüft
`autoGainRuht`/`mixRuht`/`outRuht` und lässt bei Ruhe jede Multiplikation aus (`:720`,
`:721`, `:730`), schreibt aber `L[i] = l;` zurück. In `verarbeitePfad` sind ohne Übergang nur
`memcpy` (`:778-779`) und `ausL[i] = nachL[i]` (`:808-809`) beteiligt. **Es bleiben genau
zwei Gleitkommaoperationen am Sample: `(double) float` und `(float) double`.** Der
Unity-Kurzschluss vermeidet die Multiplikation, nicht die Konversion — genau wie die
Skeptikerzeile `BEFUNDE.md:495` sagt.

### 4.2 Die Zusage, wörtlich

SONDE-015 M-02, `docs/beweise/SONDE-015.md:1035`: „Auch **ohne** Hard-Bypass ist der
engagierte, aber neutrale Kern bitidentisch: 0 dB Trim läuft **nicht** als Multiplikation
durch die Samples, sondern über einen Unity-Kurzschluss … der vollständige Weg aus §3.0 ist
durchlaufen und schreibt trotzdem keinen Sample."

Dazu `CLAUDE.md:29-30`: „Verarbeitung nur, wenn der User sie einschaltet. Ausgeschaltet ist
der Pfad im Nulltest bitidentisch; sonst Passthrough sampleidentisch, ohne Latenz oder Tail."

**Wichtige Präzisierung gegen die Befundzeile.** Die Teilaussage von M-02 „schreibt trotzdem
keinen Sample" ist am HEAD **nicht** zutreffend: der Kern schreibt im neutralen Zustand jeden
Sample (Station 3). Bitidentisch ist er trotzdem — für normale Floats geht
`float → double → float` verlustfrei zurück. Die Zusage, die tatsächlich auf dem Spiel steht,
ist deshalb **nur** „bitidentisch", nicht „schreibt keinen Sample".

**Die ausgeschaltete Zusage ist nicht betroffen.** Bei `eq_enabled = false` ist
`istPassthrough` true, `committedRuht` true und der Rückweg `:963-972` wird genommen; es wird
kein Sample geschrieben, also kann auch kein Bit kippen. Der Grundgesetzsatz „Ausgeschaltet
ist der Pfad im Nulltest bitidentisch" ist am HEAD **intakt**. Betroffen ist der Fall
„eingeschaltet, aber neutral", also die zweite Hälfte („sonst Passthrough sampleidentisch")
und M-02.

### 4.3 Heutige Abdeckung und warum sie den Befund nicht fängt

Bein A16 (`EqCopProbeeqNullTest`, Behauptungszeile `tools/beweise.ps1:688`), Fall
`eq_an_bypass_aus_alles_neutral_ist_bitidentisch`,
`eq-copilot/plugin/tests/SondeNullTestMain.cpp:356-373`.

**Das Orakel ist bereits bitgenau.** `SondeNullTestMain.cpp:167-173`:

```
            for (int k = 0; k < 2; ++k)
                if (std::memcmp (puffer.getReadPointer (k) + n, kopie.getReadPointer (k) + n, sizeof (float)) != 0)
                    { ++l.abweichend; break; }
```

Verglichen werden **Bytes**, nicht Werte. Der Fall fällt also nicht am Vergleich, sondern
ausschließlich am **Material**. `SondeNullTestMain.cpp:77-83`:

```
/** Reproduzierbares Rauschen - dieselbe Saat ergibt dieselbe Folge. */
void fuelle (juce::AudioBuffer<float>& puffer, juce::Random& wuerfel)
{
    for (int k = 0; k < puffer.getNumChannels(); ++k)
        for (int n = 0; n < puffer.getNumSamples(); ++n)
            puffer.setSample (k, n, wuerfel.nextFloat() * 1.8f - 0.9f);
}
```

Gleichverteilte normale Floats in [−0,9; 0,9). Ein subnormaler Wert (Betrag unter
1,18e-38) ist in dieser Verteilung praktisch unerreichbar, ein negatives Null ebenso. Der
einzige Fall mit besonderen Bitmustern, `passthrough_sanitisiert_nichts` (`:350-353`), fährt
NaN/±Inf — aber **ausgeschaltet und im Hard-Bypass**, also gerade nicht im betroffenen
Zustand.

**Repoweite Gegenprobe.** Kein Test am HEAD speist ein subnormales *float-Sample* durch den
Audiopfad. Was es gibt, trifft andere Sachen: `DspGoldenTestMain.cpp:3132-3163` prüft den
**Zustands**-Denormalriegel (Schwelle `kDenormalSchwelle = 1e-300`,
`eq-copilot/plugin/dsp/DspFilter.h:60-68`) — das ist eine double-Schwelle im Filterzustand,
nicht die float-Kante am Eingang; `SchemaTestMain.cpp:65` und
`StateMigrationTestMain.cpp:659-666` prüfen subnormale Zahlen im **JSON-Kanon**.

### 4.4 Urteil: PRÄZISIERT

**Was gesichert ist (aus der Quelle am HEAD):**

1. DAZ **und** FTZ sind im ganzen `processBlock` gesetzt (Station 1).
2. Der neutral eingeschaltete Kern schreibt jeden Sample zurück (Stationen 2, 3).
3. Am Sample steht dabei genau ein `float→double` und ein `double→float` (Station 4).
4. Kein Test am HEAD speist ein subnormales Sample oder ein negatives Null in diesen Zustand
   (4.3). Die **Korpuslücke ist damit sicher**, unabhängig von Punkt 5.
5. M-02 verspricht für genau diesen Zustand Bitidentität (4.2).

**Was ich nicht klären konnte:** ob `CVTSS2SD` und `CVTSD2SS` auf dieser Maschine DAZ
beziehungsweise FTZ tatsächlich anwenden. Das ist die einzige Stelle, an der die Kette eine
Ausführung braucht, und mein Auftrag verbietet Bau und Lauf. Ist die Antwort ja — was für
x86-SSE die verbreitete Lesart ist und die Phase-1-Herleitung
(`BEFUNDE.md:343`) annimmt —, dann wird `0x00000001` zu `0x00000000` und M-02 ist gebrochen:
**DEFEKT**. Ist die Antwort nein, bleibt die Korpuslücke bestehen und der Befund ist eine
**LÜCKE** (Punkt 4). In beiden Fällen ist die Arbeit dieselbe und der Rotbeweis derselbe;
**die Messung muss nur vor dem Fix laufen, nicht statt seiner.** Die Arbeitsübersicht
verlangt genau das: „aktueller vollständiger Prozessor-/Wrapperpfad mit Bitmustern nötig"
(`BEFUNDE.md:807`).

**Zwei Korrekturen an der Befundzeile**, beide ohne Wirkung auf das Urteil:

- Die Befundzeile zitiert `juce_FloatVectorOperations.cpp:1548`, die Skeptikerzeile `:1558`.
  Am HEAD steht `intptr_t mask = 0x8040;` an **drei** Stellen (`:1515`, `:1533`, `:1548`);
  der für `ScopedNoDenormals` maßgebliche Konstruktor beginnt bei `:1544`. Die Datei ist ein
  **Bauartefakt** und gehört nicht ins Repo — für einen dauerhaften Beweis darf sie nicht die
  zitierte Quelle sein.
- „ausgeschaltet" ist nicht betroffen (4.2). Die Befundzeile sagt das zwar mit
  „bei eingeschaltetem neutralem EQ", die Klasse K6/`critical` legt aber den Grundgesetzbruch
  nahe, den es hier nicht gibt.

### 4.5 Kategorie und Schwere

**DEFEKT gegen M-02, zusätzlich Korpuslücke im A16-Material — Schwere medium** (Audit:
`critical`).

Begründung der Herabstufung, zur Entscheidung durch den Dirigenten: ein subnormaler
float liegt unter −758 dBFS; musikalisch und messtechnisch ist der Verlust folgenlos.
Beschädigt wird eine **geschriebene Zusage** und ein **beglaubigendes Bein** — das ist ernst,
aber es ist kein hörbarer Produktfehler und kein Bruch des Grundgesetzsatzes zum
ausgeschalteten Pfad. Die `critical`-Einstufung des Audits folgt der Klasse K6
(Grundgesetz), nicht einer gemessenen Wirkung.

### 4.6 Kleinster Fix und Wechselwirkungen

**Ort:** `DspKern::verarbeiteStueck`, `eq-copilot/plugin/dsp/DspKern.cpp:897-972`.

**Idee.** M-02 wörtlich machen: wenn der Committed-Pfad zwar engagiert ist, aber **nichts
tut** — kein aktives Band, keine Mono-Bass-Stufe, alle fünf Rampen in Ruhe auf ihrem
neutralen Wert (`ruhtBei (1.0)`, `DspKern.h:336`), kein laufender Übergang, keine
Candidate-Rechnung —, dann denselben Rückweg nehmen wie `committedRuht` (`:963-972`): Taps
sind vorher schon gefüllt (`:948-960`), der Puffer bleibt **unberührt**. Das ist ein
Prädikat über Programm und Rampenstand, einmal je Block ausgewertet: keine Allokation, keine
Sperre, kein zusätzlicher Zweig im Samplepfad — und es entfernt den Zahlenrücklauf statt ihn
zu reparieren.

**Ausdrücklich nicht:** MXCSR für den neutralen Fall umschalten. Das änderte auch das
Verhalten des nicht-neutralen DSP und dürfte nur über die vollständige Hostgrenze beurteilt
werden (`BEFUNDE.md:807`).

**Wechselwirkungen.** Mit T3-09-02 besteht eine **gemeinsame Ursache auf der Begriffsebene**:
an beiden Stellen wird numerische Gleichheit dort geführt, wo Bitgleichheit zugesagt ist —
einmal im Produkt (der Rücklauf), einmal im Messgerät (der Vergleich). Beide gehören in
denselben Änderungssatz, und zwar in dieser Reihenfolge: **erst das Messgerät (T3-09-02),
dann das Produkt**, sonst beglaubigt ein blindes Instrument den reparierten Pfad. Keine
Verbindung zu W01 oder W03. Zu beachten: der Fix berührt den Weg, über den W01 und W03
gemessen werden (derselbe `verarbeiteStueck`-Rückweg) — deren Rotbeweise laufen mit
eingeschalteten Bändern beziehungsweise nicht ruhenden Rampen und sind vom Prädikat nicht
betroffen.

### 4.7 Ort des Rotbeweises

**Bein A16** (`tools/beweise.ps1:688`), `eq-copilot/plugin/tests/SondeNullTestMain.cpp`,
Fall `eq_an_bypass_aus_alles_neutral_ist_bitidentisch` bei `:356-373`.

Das Orakel (`:167-173`, `std::memcmp`) bleibt wie es ist — **nur das Material ändert sich.**
Neben `fuelle` (`:77-83`) tritt ein fester Bitmusterblock, der in jeden Lauf des M-02-Falls
eingestreut wird: `0x00000001` (kleinster positiver Subnormal), `0x80000001`, `0x007FFFFF`
(größter Subnormal), `0x807FFFFF`, `0x00800000` (kleinster Normal) und seine Negation,
`0x00000000` und `0x80000000` (±0). Assertion: `lauf.abweichend == 0` — heute rot, sobald
die Konversion einen dieser Werte verändert, und nach dem Fix grün, weil kein Sample mehr
geschrieben wird.

Gegenfälle im selben Fall: dieselben Bitmuster bei `eq_enabled = false` (muss heute schon
grün sein — Beleg, dass die ausgeschaltete Zusage intakt ist) und im Hard-Bypass; dazu ein
Lauf mit **wirksamem** Band, der zeigen muss, dass der neue Rückweg dort **nicht** greift.
Die Behauptungszeile `tools/beweise.ps1:688` muss die Bitmuster ausdrücklich nennen.

---

## 5. T3-09-02 — der Host-Nulltest urteilt BITIDENTISCH nach Zahlengleichheit (W04)

*Befundzeile: `BEFUNDE.md:467` · Skeptikerzeile Phase 10: `:545` · Arbeitsübersicht W04:
`:807`*

### 5.1 Quellkette, zitiert

**Station 1 — was das Werkzeug über sich selbst behauptet.** `tools/fl/nulltest.py:20-22`:

```
volle Songlaenge (Kreuzkorrelation beider Kanaele, |v| < N), Vergleich nach
Ausrichtung ueber die ueberlappenden Renderframes max(0, v) bis
min(N, Quellframes + v) - 1 auf der exakten Float-32-Darstellung beider Dateien.
```

und `:28-29`:

```
Urteil und Szenario-Exit (F-19, F-24): BITIDENTISCH 0 (v = 0, 0 Abweichungen) ·
```

**Station 2 — was es tatsächlich vergleicht.** `tools/fl/nulltest.py:206-207`:

```
    ungleich = r != q
    abweichungen = int(np.count_nonzero(ungleich))
```

`r` und `q` sind float32-Arrays (`:205-206`, `np.ascontiguousarray`). `r != q` ist
**numerische** Gleichheit. Für `+0.0` gegen `-0.0` ist sie erfüllt; die Bitmuster
`0x00000000` und `0x80000000` sind es nicht.

**Station 3 — die Hashes werden gerechnet und liegen ungenutzt daneben.**
`tools/fl/nulltest.py:212-213`:

```
        "sha256_render_bereich": hashlib.sha256(r.astype("<f4").tobytes()).hexdigest().upper(),
        "sha256_quelle_bereich": hashlib.sha256(q.astype("<f4").tobytes()).hexdigest().upper(),
```

**Station 4 — das Urteil fällt vor jedem Hashvergleich.** `tools/fl/nulltest.py:225-228`:

```
    if abweichungen == 0:
        ergebnis["urteil"] = "BITIDENTISCH" if v == 0 else "VERSATZ"
        if v == 0:
            ergebnis["g"], ergebnis["g_db"] = 1.0, 0.0
        return ergebnis
```

Der `return` liegt vor jeder Verwendung der beiden Hashes. `:252` bildet das auf den
Exitcode ab: `EXIT_JE_URTEIL = {"BITIDENTISCH": EXIT_OK, …}`.

**Station 5 — das Werkzeug kann es bereits richtig, nur woanders.** Der `ohne_slots`-Weg
(R2) vergleicht sehr wohl byteweise, `tools/fl/nulltest.py:454`:

```
    gleich = sha == a["sha256_render_bereich"]
```

Die Fähigkeit ist also vorhanden; sie fehlt genau am R1-Urteil.

**Wo R1 gefahren wird.** `docs/gesundheit/szenarien/nulltest-host.json:3` trägt den Titel
„Der Render des Diagnoseprojekts im Auslieferungszustand ist **bitidentisch** zur Quelle
(Weg R1) …", Schritt `:12` ist `{"aktion": "lokal.nulltest", "params": {"vergleich":
"auslieferung"}}`. Der Aufruf läuft über `tools/fl/szenario.py:985-988`.

### 5.2 Die Zusage, wörtlich

- `CLAUDE.md:30`: „Ausgeschaltet ist der Pfad im Nulltest bitidentisch; sonst Passthrough
  sampleidentisch, ohne Latenz oder Tail."
- Das Werkzeug selbst, `tools/fl/nulltest.py:22`: „… auf der exakten Float-32-Darstellung
  beider Dateien."
- Das Szenario, `docs/gesundheit/szenarien/nulltest-host.json:3`: „… ist **bitidentisch** zur
  Quelle (Weg R1)".

Alle drei sagen „bitidentisch"; Station 2 misst Zahlengleichheit. Der Widerspruch steht in
derselben Datei wie die Behauptung.

### 5.3 Heutige Abdeckung und warum sie den Befund nicht fängt

Bein A35 (`selbsttest.py`, Behauptungszeile `tools/beweise.ps1:642`) ruft nach
`tools/fl/selbsttest.py:40` die Selbstprüfung des Nulltests auf
(`nulltest.py --selbsttest`). Der einschlägige Fall ist
`fall_identisch_null_abweichungen`, `tools/fl/nulltest.py:615-637`:

```
    k = 123_457
    r = q[:N_TEST].copy()
    r[k, 0] = q[k, 0] + np.float32(2.0 ** -23)
```

Perturbiert wird ein **LSB nach oben** — ein Wertunterschied, den auch die numerische
Gleichheit sieht. Kein Fall im ganzen Modul setzt ein negatives Null; die Zeichenfolge `-0.0`
kommt in `tools/fl/nulltest.py` nicht vor. Die Behauptungszeile `tools/beweise.ps1:642` sagt
zum Nulltest: „der Nulltest trennt Format, Versatz beider Vorzeichen, Kettenfaktor und
Abweichung" — Vorzeichen bezieht sich dort auf das **Versatzvorzeichen**, nicht auf das
Vorzeichen der Null.

### 5.4 Urteil: BESTÄTIGT

Die Kette trägt vollständig: Behauptung (Station 1) gegen Messung (Station 2), Hashes
vorhanden aber ungenutzt (Stationen 3, 4), und die Selbstprüfung deckt den Fall nicht
(5.3). Bestätigt wird — wie schon die Skeptikerzeile `BEFUNDE.md:545` festhält — eine
**falsche Detektoraussage**, kein beobachteter Nulltestbruch des Plugins.

**Eigene Ergänzung zur Erreichbarkeit** (Herleitung aus der Quelle, **nicht gemessen**): ein
Vorzeichenwechsel der Null ist im Produkt nicht rein hypothetisch. In der M/S-Stufe
(`eq-copilot/plugin/dsp/DspKern.cpp:675-690`) gilt für `L[i] == R[i] == -0.0`:
`m = (L+R)*0.5 = -0.0`, `s = (L-R)*0.5 = (+0.0)*0.5 = +0.0`, und `L[i] = m + s` ist nach
IEEE-754-Rundung `-0.0 + 0.0 = +0.0`. Ein bloßer `width`-Wert ungleich 1,0 — für ein
L=R-Signal sonst wirkungslos — kann also ein `-0.0` in ein `+0.0` verwandeln. Für den
**ausgeschalteten** Pfad kann das nicht passieren, weil dort kein Sample geschrieben wird
(`DspKern.cpp:963-972`, siehe 4.2). Der Befund betrifft damit die Beweiskraft für die zweite
Hälfte des Grundgesetzsatzes.

### 5.5 Kategorie und Schwere

**DEFEKT des Beweiswerkzeugs, medium.** Ein Bein, das „bitidentisch" beglaubigt, ohne Bits zu
vergleichen, stellt ein falsches Zeugnis aus — das ist ein Defekt, auch wenn das Produkt
heute nicht darunter leidet. Gegen „high" spricht, dass die Fehlaussage nur in eine Richtung
geht (ein echter Wertunterschied wird stets gesehen) und dass der Unterschied, den sie
übersieht, hörbar folgenlos ist.

### 5.6 Kleinster Fix und Wechselwirkungen

**Ort:** `vergleiche`, `tools/fl/nulltest.py:194-250`.

**Idee.** Den ausgerichteten Bereich über seine uint32-Bitmuster vergleichen, so wie R2 es
über den SHA-256 schon tut (Station 5): `ungleich = r.view("<u4") != q.view("<u4")` statt
`r != q` in `:206` — damit fallen `+0` und `−0` auseinander, und `NaN` gegen sich selbst wird
richtig als gleich gewertet statt als Abweichung. Die nachgelagerte Kettenrechnung (`:230-247`)
muss dann weiterhin auf den Zahlen arbeiten, damit ein gekippter Nullvorzeichenwert nicht als
„Kette mit Faktor g" durchgeht — also: Bitvergleich für Zählung und Urteil, Zahlenvergleich
für die Erklärung. Alternativ genügt als Minimalfassung ein Gleichheitscheck der beiden schon
gerechneten Hashes (`:212-213`) vor dem `return` in `:228`. Kein Echtzeitbezug, reines
Python-Werkzeug.

**Wechselwirkungen.** Gemeinsame Ursache mit T3-01-01 (siehe 4.6): dieselbe Verwechslung von
Zahlengleichheit und Bitgleichheit, einmal im Messgerät und einmal im Produkt. Reihenfolge:
**dieses Werkzeug zuerst.** Zu beachten: nach dem Fix kann der Laufzeit-Arm an
`docs/gesundheit/szenarien/nulltest-host.json:12` aus Gründen rot werden, die nicht am Plugin
liegen (Nullvorzeichen aus dem Host- oder Renderweg); das ist dann ein echter Befund und darf
nicht als Regression des Plugins gelesen werden. Keine Verbindung zu W01 oder W03.

### 5.7 Ort des Rotbeweises

**Bein A35** (`tools/fl/selbsttest.py`, Behauptungszeile `tools/beweise.ps1:642`), Datei
`tools/fl/nulltest.py`, Fall `fall_identisch_null_abweichungen` bei `:615-637` — direkt neben
der vorhandenen LSB-Probe.

Aufbau: Quelle und Render identisch bis auf einen einzigen Frame, an dem die Quelle `+0.0`
und der Render `-0.0` trägt (beide als gültiges Float-32-WAV geschrieben, `v = 0`).
Assertionen:

- `urteil == "ABWEICHUNG"` und Exit 4 — **heute `BITIDENTISCH` und Exit 0**, also rot;
- `abweichungen == 1` mit `erste["frame"] == letzte["frame"] == k`;
- `sha256_render_bereich != sha256_quelle_bereich` (belegt, dass die Bytes wirklich
  auseinandergehen — hält schon heute und macht sichtbar, dass die Information vorlag);
- Gegenfall: wirklich identische Dateien bleiben `BITIDENTISCH` mit Exit 0 (hält heute, muss
  halten);
- Gegenfall: ein `NaN` an derselben Stelle in beiden Dateien darf nach dem Fix **nicht** als
  Abweichung zählen (heute zählt es als eine, weil `NaN != NaN`) — sonst repariert der Fix
  eine Blindheit und erzeugt eine Falschmeldung.

Die Behauptungszeile `tools/beweise.ps1:642` muss das Vorzeichen der Null ausdrücklich
nennen; heute steht dort nur „Versatz beider Vorzeichen", was etwas anderes meint.

---

## 6. Zusammenfassung für die Einordnung

**Zwei gemeinsame Ursachen, drei getrennte Änderungssätze.**

1. **W01 und W03 teilen eine Wurzel, aber nicht den Fix.** `DspKern::blockrand`
   (`eq-copilot/plugin/dsp/DspKern.cpp:357-425`) entscheidet ausschließlich aus den zwei
   Programmen, die es im Augenblick sieht. Was in der Ruhe geschah (W01) oder was als
   Zwischenpublikation verdrängt wurde (T3-14-02), ist dort unsichtbar. Die Reparaturen sind
   trotzdem getrennt: W01 betrifft fünf Skalare je **Pfad**, W03 den Zustand je **Slot** einer
   **Bank**. `BEFUNDE.md:806` verlangt ausdrücklich, W03 nicht über W01 zu lösen; die
   Quellenlektüre bestätigt das.
2. **T3-14-02 und T3-15-08 sind eine Sache in zwei Richtungen** und müssen zusammen entschieden
   und zusammen gemessen werden: es fehlt eine Slot-Lebenszykluskennung samt je Slot
   definierter Kompatibilität. Jede Pauschalreparatur repariert eine Richtung und verschärft
   die andere.
3. **T3-01-01 und T3-09-02 teilen einen Begriffsfehler**: numerische Gleichheit dort, wo
   Bitgleichheit zugesagt ist — einmal im Produktpfad, einmal im Messgerät. Reihenfolge:
   Messgerät zuerst.

**Was ich nicht prüfen konnte.**

- **Ob `CVTSS2SD`/`CVTSD2SS` auf dieser Maschine DAZ/FTZ anwenden** (4.4). Das entscheidet, ob
  T3-01-01 DEFEKT oder LÜCKE ist. Die Arbeit und der Rotbeweis sind in beiden Fällen dieselben;
  die Messung gehört als erster Schritt des Tickets vor den Fix. Kein Bau und kein Lauf war
  erlaubt.
- **Alle Auditmessungen als solche.** +13,41 dB habe ich aus der Quelle unabhängig
  nachgerechnet und exakt getroffen (1.4) — das ist Rechnung, nicht Lauf. Die Werte
  `dynamicDb=-12`, `audioPeak=0.1025077403` (T3-14-02) und `−8,002865 dB` / 200 ms
  (T3-15-08) stammen aus den Phase-16-Läufen der Pakete D und F; ich habe ihre Quellketten
  verifiziert, nicht ihre Zahlen reproduziert.
- **Der Hostweg.** Kein FL-Lauf, kein VST3-Wrapper, keine Installation. Die Erreichbarkeit von
  T3-15-05 über Hostautomation ist an der Quelle hergeleitet (1.1, Station „Erreichbarkeit"),
  nicht im Host gemessen; W01 stellt den Hostnachweis ohnehin hinter V1
  (`BEFUNDE.md:804`).
- **Die übrigen sechs IDs des Tickets** (T3-15-07 in W07, T3-15-09 und T3-16-04 in W08,
  T3-15-06, T3-15-10, T3-15-11 in W35) waren nicht Gegenstand dieses Teils.
