# NAK-311 — Quellvalidierung Teil B (lesender Opus-Agent, HEAD 7463c3eb)

**Ticket:** NAK-311 · Planschritt S25j „Tiefenaudit 3 abarbeiten, Teil 3: Audio-Kern" ·
Etappe 0 „Quellvalidierung", **Teil B**
**HEAD:** `7463c3ebe78c2637bec9da42e23d7f6f2bd9af38` (Zweig `master`)
**Datum:** 19.09.2026 · **Modell:** Opus (lesend)
**Gegenstand (6 Befunde + 2 Abdeckungsfragen):** T3-15-07 (W07); T3-15-09 und T3-16-04 (W08);
T3-15-06, T3-15-10, T3-15-11 (W35); F08 (Niedrigratensupport) und F12 (warmer Recall)
**Quelle der Befunde:** `docs/audits/2026-09-15-tiefenaudit/BEFUNDE.md`
(Paket-2-Tabelle ab `:627`, Gegenprüfungstabelle ab `:648`, Phase-16-Tabelle ab `:714`,
Abschlussstatus der Felder ab `:742`, Arbeitsübersicht ab `:794`) und die Rohberichte
`roh/phase-15-paket2-dsp-numerik.md`, `roh/phase-15-gegenpruefung-paket2.md`,
`roh/phase-16-d-dsp.md`, `roh/phase-16-abdeckung.md`.
**Muster:** `docs/beweise/roh/NAK-309-quellvalidierung-teil-A.md`.

**Prüfform:** ausschließlich Quellenlektüre am HEAD. Kein Bau, kein Testlauf, kein Kanon,
kein FL Studio, kein Codex, keine Fehlerinjektion. Git nur lesend. Jede Zeilennummer unten
ist am HEAD `7463c3eb` selbst nachgelesen. Legacy-Bezeichner (`EQ-Copilot`, `EqCop*`,
`Eqcp`) sind kein Befund. Der Designteil ist geparkt (12.09.2026): unten steht nur
Funktions- und Klangverhalten, keine Optik.

**Arbeitsbaum, geprüft mit `git status --short`.** *Zu Beginn des Laufs:* nur die zwei
fremden untracked Ordner `briefing-hub/` und `nimbalyst-local/`, sonst nichts; HEAD
`7463c3eb`. *Beim Schreiben dieser Datei:* unverändert dieselben zwei Ordner, HEAD weiterhin
`7463c3eb`. Der HEAD ist während des Laufs **nicht** gewandert; ein Basisabgleich war nicht
nötig. Diese Datei ist die einzige Schreibaktion des Agenten.

**Stand des Audits gegen den HEAD.** Die Phase-16-Berichte prüften
`aff2d8188f33a9525aec1869449773bfdbe305a6`.
`git diff --stat aff2d818 7463c3eb -- eq-copilot broker tools` berührt **keine** der unten
zitierten Produktdateien: `eq-copilot/plugin/dsp/**`, `eq-copilot/plugin/sonde/**`,
`eq-copilot/plugin/state/**`, `eq-copilot/plugin/tests/DspGoldenTestMain.cpp`,
`eq-copilot/plugin/tests/TransactionTestMain.cpp`, `eq-copilot/plugin/tests/SchemaTestMain.cpp`,
`eq-copilot/schemas/**` stehen nicht in der Diff-Liste. Verändert wurden dort nur
`tools/beweise.ps1`, `tools/fl/**`, `tools/eq-copilot/pruefe_*.py`, `broker/**`,
`plugin/src/**`, `plugin/probe/**` und vier andere Testdateien. **Alle Zitate der Phase 16
gelten am HEAD unverändert**; die Zeilennummern in `tools/beweise.ps1` sind unten frisch am
HEAD gelesen, nicht aus dem Audit übernommen.

---

## Ergebnis in einer Zeile je ID

| ID | Urteil | Kategorievorschlag | Schwere | Ort des Rotbeweises | Produktentscheid nötig |
|---|---|---|---|---|---|
| T3-15-07 (W07) | BESTÄTIGT, präzisiert in der Erreichbarkeit (`freq_hz` ist Host-Parameter → Stufe 1 statt 2) | LÜCKE | medium | Bein B6 `EqCopDspGoldenTest`, neuer Fall neben `DspGoldenTestMain.cpp:1482-1552`; Behauptungszeile `tools/beweise.ps1:723` | **ja** — U44 trifft, Optionstext ist zu korrigieren |
| T3-15-09 (W08) | BESTÄTIGT, präzisiert: zwei trennbare Teile (Vertragsherleitung / Audiofolge) | LÜCKE (Teil a HÄRTUNG) | low; Teil b medium bei bejahtem Deckel | Beine B6 (`DspGoldenTestMain.cpp:2433-2621`, Behauptung `tools/beweise.ps1:723`) und B3c `EqCopSchemaTest` (`tools/beweise.ps1:702`) gegen `$defs/dsp_bericht` | **ja, nur Teil b** — keine Karte vorhanden, U44-U47 treffen nicht |
| T3-16-04 (W08) | BESTÄTIGT, präzisiert: es gibt eine datierte Produktabsicht (Abnahme 24.08.2026), die heute im Monobus verfehlt wird | LÜCKE | medium | Bein B6, neuer Mono-Abschnitt neben `DspGoldenTestMain.cpp:2433-2621`; Gegenwache Bein A16 (`tools/beweise.ps1:688`) | **nein** — Abnahme 24.08.2026 plus R4 legen die Regel fest |
| T3-15-06 (W35) | BESTÄTIGT (Herabstufung der Gegenprüfung trägt) | LÜCKE | low | Bein B6, neuer Fall neben `DspGoldenTestMain.cpp:1691-1728`; Behauptungszeile `tools/beweise.ps1:723` | **ja** — U45 trifft |
| T3-15-10 (W35) | BESTÄTIGT als Härtung | HÄRTUNG | low | nur bei Antwort „einführen": Bein B6, C-Abschnitt `DspGoldenTestMain.cpp:1121-1177`; sonst kein Codebeweis, nur Registerzeile | **ja** — U46 trifft, Kostenhinweis fehlt |
| T3-15-11 (W35) | BESTÄTIGT (Arithmetik am HEAD nachgerechnet) | HÄRTUNG | low | Bein B6, neuer Fall neben `DspGoldenTestMain.cpp:1972-2059` (angewandte Verstärkung aus dem Tap); Behauptungszeile `tools/beweise.ps1:723` | **nein** — reine Begründungs- und Nachweiskorrektur |
| F08 Niedrigratensupport | BESTÄTIGT als Abdeckungsfrage, präzisiert: R4 nennt 44,1 kHz ausdrücklich als kleinste unterstützte Rate | LÜCKE | low | Beine A16 (`tools/beweise.ps1:688`) und B6 (`:723`) mit einem 32-kHz-Fall | **ja** — U47 trifft |
| F12 warmer Recall | PRÄZISIERT (pauschaler Kaltstart widerlegt; offen sind Referenzbeginn und Toleranz von M-84) | LÜCKE | low | Bein B7 `EqCopTransactionTest`, neuer Fall neben `TransactionTestMain.cpp:2170-2191`; Behauptungszeile `tools/beweise.ps1:728` | **nein** — Invarianten und M-84 geben die Regel vor |

---

## 1. T3-15-07 — Übergangsspitze bei großen Wertesprüngen (W07)

*Befundzeile: `BEFUNDE.md:633` · Gegenprüfung: `roh/phase-15-gegenpruefung-paket2.md:164-221` (K3) ·
Finder: `roh/phase-15-paket2-dsp-numerik.md:209-231` (P15-D-03) · Arbeitsübersicht: `BEFUNDE.md:810`*

### 1.1 Quellkette, zitiert

**Station 1 — die Entscheidung Rampe oder Crossfade.** `eq-copilot/plugin/dsp/DspKern.cpp:397-414`
in `DspKern::blockrand` (Funktionskopf `:357`):

```
    auto& bankNeu = baenke.bank (neu);
    const bool nurRampen = alt >= 0 && rampenKompatibel (baenke.bank (alt).programm, bankNeu.programm);

    if (nurRampen)
    {
        bankNeu.baender         = baenke.bank (alt).baender;
        bankNeu.monoBassZustand = baenke.bank (alt).monoBassZustand;
    }

    if (alt >= 0) baenke.beginneVerblassen (alt);
    z.quelle    = alt;
    z.aktiv     = neu;
    z.uebergang = nurRampen ? Uebergang::rampe : Uebergang::crossfade;
    z.rest      = nurRampen ? kRampeSamples : kFadeSamples;
```

**Station 2 — `rampenKompatibel` kennt keine Wertegrenze.** `eq-copilot/plugin/dsp/DspProgramm.cpp:139-159`
prüft ausschließlich Topologie: `eqEngagiert`, `hardBypass`, `samplerate`, Mono-Bass an/aus und je Slot
`aktiv`, `typ`, `modus`, `dynamisch`, `nutztSvf`, `quelle`. `freq_hz`, `q` und `gain_db` kommen darin nicht vor.

**Station 3 — was während der Rampe gerechnet wird.** `eq-copilot/plugin/dsp/DspKern.cpp:493-495`
(Rampenposition je Sample) und `:588`:

```
        const Biquad f = mitte ? mische (qb->statisch, b.statisch, t) : b.statisch;
```

`mische(Biquad)` steht bei `:43-52`; die Stabilitätsbegründung E-19 im Kommentar `:39-42`
(„jeder Punkt auf der Strecke zwischen zwei stabilen Entwuerfen ist selbst stabil").
Die Gegenprüfung hat die Zwischenentwürfe nachgerechnet und die Stabilitätsaussage **bestätigt**
(`roh/phase-15-gegenpruefung-paket2.md:170-175`) — der Ausschlag ist kein Stabilitätsfehler,
sondern das Ausklingen der angeregten tiefen Polstelle (Spitze bei Sample 444, also **nach**
dem Rampenende bei 256, `:213-219`).

**Station 4 — die Auslöser sind gebaut und teils Stufe 1.** `freq_hz` ist ein
**Host-Parameter** mit `wechsel = rampe`: `eq-copilot/schemas/state/nakama-parameter-v2.json:153-165`
(`"min": 20.0`, `"max": 20000.0`, `"wechsel": "rampe"`, `"host_parameter": true`). Eine einzelne
Automationsstufe 5 kHz → 50 Hz ist damit am Hosteingang erreichbar, sobald der Slot belegt ist.
Die Belegung selbst ist **nicht** automatisierbar (`:300-308`, `"host_parameter": false`,
Begründung im Feld `bedeutung`), kommt aber über Presetladen, geladenen Zustand, `apply`,
Undo/Redo aus `NakamaTransaktion.cpp:515-576` (`Art::apply`, `Art::presetLaden`,
`Art::bandBelegen`, `Art::revert`, `Art::undo`, `Art::redo`).

**Gemessene Größenordnung (Altbasis, unabhängig nachgerechnet, hier nicht erneut ausgeführt):**
Low-Shelf +6 dB Q 0,707, 48 kHz, nur `freq_hz` 5 kHz → 50 Hz, 1-kHz-Sinus 0,5: **+17,41 dB**
über der größeren stationären Spitze; Low-Cut 2 kHz → 20 Hz: **+22,20 dB**. Derselbe Sprung über
den vorhandenen topologischen Crossfade: **−0,22 / +0,13 dB**. Zwanzig Stufen statt einer:
**+0,02 dB** (`roh/phase-15-gegenpruefung-paket2.md:180-201`).

### 1.2 Die Zusage

**Keine Zusage für die Übergangsspitze gefunden.** Die beiden nächstliegenden Kandidaten halten
beide und sind **nicht** verletzt:

`docs/FL-Nakama-Sonden-Design-Entwurf.md:2990-2991`:

> „Der Audiothread liest ein unveränderliches `DspProgram`, allokiert und sperrt nie. Stetige
> Parameter werden geglättet; Bandtyp, Kanalmodus oder Bandanzahl wechseln per kurzem Crossfade"

`docs/FL-Nakama-Sonden-Design-Entwurf.md:3109`:

> „Automations-Zipperresiduen bleiben im definierten Ramp-Test unter −100 dBFS."

Die −100 dBFS sind ausdrücklich an den **definierten** Ramp-Test gebunden, und geglättet **wird**.
E-19 begründet die lineare Mischung allein mit Stabilität, und diese Begründung trägt.

### 1.3 Heutige Abdeckung

- **Das Bein, das dasselbe misst, kann es nicht sehen:** `DspGoldenTestMain.cpp:1482-1552`
  (Fall `bandwert_<feld>_rampt_ohne_zustandsreset (M-17, R8, B-4)`, Assertion `:1545-1547`).
  Die Referenz des Tests ist **derselbe** DF2T-Biquad mit **derselben** linearen
  Koeffizientenmischung (`:1529-1541`). Der Test beweist „der Kern rechnet, was die Formel sagt" —
  nicht „die Formel erzeugt keine Spitze". Ein Residuum gegen sich selbst wird nie rot.
- **Die geprüften Sprünge sind klein:** `DspGoldenTestMain.cpp:1490-1495`:
  `gain_db` 3 → 9, `freq_hz` 1000 → 1400, `q` 1 → 3 — alle drei am Bell.
- **Behauptungszeile des Beins:** `tools/beweise.ps1:723` (B6) sagt nichts über Sprunggrößen
  oder Übergangsenergie.
- **Register:** keine Zeile. T3-14-02 sitzt an derselben Stelle (`DspKern.cpp:397-408`),
  betrifft aber den Lebenszyklus, nicht die Koeffizientenbahn.
- **Absichtskommentar, der ausdrücklich erlaubt:** keiner; `DspKern.cpp:39-42` begründet nur
  die Stabilität.

### 1.4 Urteil

**BESTÄTIGT**, mit einer Präzisierung: die Befundzeile führt „Belegtes Band Stufe 2/State,
spätere Bedienung Stufe 3" (`BEFUNDE.md:810`). Tatsächlich ist die **Wertänderung** schon heute
Stufe 1, weil `freq_hz` ein Host-Automationsparameter ist; nur die **Belegung** braucht
Transaktion oder geladenen Zustand. Für einen Musiker, der ein Projekt mit belegtem Band lädt
und dann eine Automationsstufe fährt, ist der Fall damit heute erreichbar.

**Kategorievorschlag: LÜCKE** (keine Zusage verletzt, fehlende Regel), **Schwere medium** —
gemessene +17 bis +22 dB über beiden Endzuständen, auslösbar über gewöhnliche
Produktionshandgriffe. Klasse K3 mit K5-Anteil.

### 1.5 Kleinster Fix und Ort des Rotbeweises

**Fix.** Kein neues DSP-Verfahren bauen, sondern den **vorhandenen** Mechanismus greifen lassen:
`rampenKompatibel` (`DspProgramm.cpp:139-159`) um ein Wertekriterium ergänzen — je Slot ein
Frequenzverhältnis, eine Q-Spanne und eine Gain-Differenz als Technikkonstanten mit Golden
(Muster `kFadeSamples`/`kRampeSamples`, `DspProgramm.h:50-57`). Reißt ein Band das Kriterium,
liefert die Funktion `false`, und `DspKern::blockrand` (`:413-414`) wählt den bereits gebauten
`Uebergang::crossfade` über `kFadeSamples` — den Weg, der denselben Sprung nach der
Gegenprüfung mit ≈ 0 dB löst. Die Entscheidung fällt am Blockrand im schon bestehenden Pfad,
ohne Allokation, ohne Sperre, ohne zweiten Filterlauf im Dauerbetrieb.

**Abhängigkeit, die mitgedacht gehört:** der Crossfade startet die neue Bank kalt
(E-8, `SONDE-015.md:3400`). Genau das ist der Boden von T3-14-02 und T3-15-08 (W03). Der
Fix für W07 darf deshalb erst greifen, wenn W03 die Slot-Historie geregelt hat — sonst tauscht
er eine Spitze gegen einen Einbruch. `BEFUNDE.md:810` führt diese Abhängigkeit bereits.

**Rotbeweis.** Bein **B6** `EqCopDspGoldenTest`, neuer Fall im Rampenabschnitt neben
`DspGoldenTestMain.cpp:1482-1552`: Low-Shelf +6 dB Q 0,707, 48 kHz, `freq_hz` 5 kHz → 50 Hz,
1-kHz-Sinus 0,5, 0,5 s Tap nach dem Wechsel; Erfolgskriterium
`Spitze ≤ max(stationär vorher, stationär nachher) + 1 dB`. **Heute rot** (+17,41 dB).
Gegenfälle im selben Fall: Aufwärtssprung, derselbe Sprung als dynamisches Band mit Range 0,
derselbe Sprung in 20 Stufen (muss grün bleiben, +0,02 dB), Low-Cut 2 kHz → 20 Hz.
Die Zusage trägt danach die Behauptungszeile `tools/beweise.ps1:723`, die um einen Satz über
die zulässige Übergangsspitze zu ergänzen ist.

### 1.6 Trennung Technik / Produkt

**Ohne Antwort baubar:** die Sprunggrenze selbst (ab wann ein Sprung „groß" ist), die Länge des
Crossfades, die Kapselung in `rampenKompatibel`, der Golden und die Reihenfolge gegenüber W03.

**Hängt an einem Produktentscheid:** ob bei einem großen Sprung überhaupt überblendet werden soll.
**Karte U44 trifft den Befund** (`docs/plan/fragen.json`, Titel „Großer EQ-Sprung: weich überblenden
oder sofort umschalten?", Status `offen`, seit 18.09.). Sie benennt richtig, dass die Sprunggrenze
Technik bleibt.

**Eine Ungenauigkeit im Kartentext, die der Dirigent vor dem Stellen berichtigen sollte:**
U44 beschreibt Weg 2 als „Sofort umschalten und die kurze Spitze in Kauf nehmen" und sagt im
Feld `warum`, ohne Entscheid bleibe es beim heutigen Verhalten. Beides zusammen ist irreführend:
heute wird **weder** überblendet **noch** sofort umgeschaltet, sondern die Koeffizienten werden
über 256 Samples gemischt — und dieser Weg ist im gemessenen Shelf-Fall mit +17,41 dB **schlechter**
als ein Sofortsprung (+6,85 dB). Ehrlich sind drei Optionen: (1) überblenden, (2) alles lassen wie
heute, (3) sofort umschalten. Weg 3 klickt hörbar und ist fachlich der schlechteste.

**Wirkung in Musikersprache und Empfehlung:** Wenn ein Band in einem Schritt weit springt — beim
Laden einer Voreinstellung, beim Tippen eines Werts, bei einer Automationsstufe —, kann für den
Bruchteil einer Sekunde ein deutlich lauteres Wummern oder Zischen durchkommen, das weder vorher
noch nachher im Klang steckt. Gemessen sind bis zu 17 bis 22 dB darüber, also klar hörbar, auf
einem Master auch potenziell gefährlich für Boxen. **Empfehlung: Weg 1 (überblenden).** Der Kern
kann das heute schon für Typwechsel, es kostet für den Moment des Sprungs einen zweiten
Filterlauf, und die Messung zeigt dafür ≈ 0 dB Überschuss. Langsame Regler und weiche Fahrten
bleiben davon unberührt und klingen exakt wie heute.

---

## 2. T3-15-09 — Auto-Gain: ungeklemmte Extremkurven und falsch hergeleitete Vertragsgrenze (W08)

*Befundzeile: `BEFUNDE.md:635` · Gegenprüfung: `roh/phase-15-gegenpruefung-paket2.md:280-319` (K5) ·
Finder: `roh/phase-15-paket2-dsp-numerik.md:257-269` (P15-D-05) · Arbeitsübersicht: `BEFUNDE.md:811`*

### 2.1 Quellkette, zitiert

**Station 1 — die Ableitung klemmt nicht.** `eq-copilot/plugin/dsp/DspProgramm.cpp:132-135`:

```
    const double mittel = summe / (double) kAutoGainStellen;
    if (! (mittel > 0.0) || ! std::isfinite (mittel)) return 0.0;
    if (mittel == 1.0) return 0.0;   // bitgenau flach: log10(1,0) ist 0, aber -0,0 ist es nicht
    return -10.0 * std::log10 (mittel);
```

Geprüft wird ausschließlich „größer null und endlich". Der Kurzschluss bei leerem Programm steht
bei `:96`. Der Wert wird bei `:266-267` in `autoGainDb`/`autoGainLin` übernommen.

**Station 2 — der Bericht übernimmt ihn ungeklemmt.** `eq-copilot/plugin/state/NakamaTransaktion.cpp:796`
in `baueBericht` (Funktionskopf `:773`):

```
        aus.autoGainDb = prog->autoGainDb;
```

**Station 3 — die Vertragsgrenze mit ihrer Herleitung.**
`eq-copilot/schemas/v3/eq-ipc-v3.schema.json:2324-2329`:

```
        "auto_gain_db": {
          "$comment": "ABGELEITET, nie persistent und nie im state_hash (R4). ...
          Die Grenze folgt aus dem Erreichbaren: acht Baender zu je 12 dB im selben
          Kanalmodus ergeben hoechstens 96 dB Plateau; 120 ist die Reserve darueber.",
          "type": "number",
          "minimum": -120,
          "maximum": 120
```

Dieselbe Herleitung steht als Entscheid E2-7, `docs/beweise/SONDE-015.md:2951`:

> „Acht Bänder zu je 12 dB im selben Kanalmodus ergeben höchstens 96 dB Plateau; 120 ist die Reserve."

**Station 4 — die Anwendung im Audio.** `DspKern.cpp:424` setzt das Rampenziel
(`z.rampen.autoGain.setzeZiel (pn.autoGainAn ? pn.autoGainLin : 1.0)`), `:715` und `:720`
wenden es je Sample auf beide Puffer an. Der Schalter `v2.global.auto_gain` ist ein
**Host-Parameter** (`eq-copilot/schemas/state/nakama-parameter-v2.json:113-122`,
`"host_parameter": true`; Vertragsreihenfolge `:427`).

**Station 5 — der Sender ist heute nicht angeschlossen.** `SondeProcessor::dspBericht`
(`eq-copilot/plugin/sonde/SondeProcessor.cpp:1281-1285`) ist der einzige Produktaufrufer von
`baueBericht`; sein einziger Aufrufer im Baum ist `TransactionTestMain.cpp:2200`. Ein
Drahtvertragsbruch ist heute damit nicht auslösbar — die **Audiofolge** dagegen schon.

**Gemessene Größenordnung (Altbasis, unabhängig bestätigt, hier nicht erneut ausgeführt):**
acht Low-Shelves +12 dB Q 8 bei 1 kHz → **−199,77 dB**; Q 24 → −275,52 dB;
acht High-Cuts 20 Hz Q 0,15 → **+150,46 dB**; acht High-Cuts 20 Hz Q 0,707 → **+42,99 dB**;
acht Shelves Q 0,707 bleiben mit −92,29 dB innerhalb (`roh/phase-15-gegenpruefung-paket2.md:307-312`).
Alle Werte liegen im Vertragsbereich: `q` 0,15 bis 24 für alle Typen, `gain_db` ±12 dB.

### 2.2 Die Zusage

**Keine Zusage zur Klemmung gefunden.** R4 (`docs/beweise/SONDE-015.md:1603-1616`) und die
Matrixzeilen M-35 bis M-40 (`:1083-1088`) nennen keine Grenze für den abgeleiteten Wert.

**Eine Zusage gibt es trotzdem, und sie ist die Grenze selbst:** `auto_gain_db` in ±120 dB
(E2-7, `SONDE-015.md:2951`, und `eq-ipc-v3.schema.json:2327-2328`). Sie ist nicht verletzt,
solange kein Produktsender existiert — aber ihre **Herleitung** ist nachweislich falsch:
RBJ-Shelves überschwingen bei Q > 0,707, und Cuts dämpfen unbegrenzt; das Erreichbare ist
größer als 96 dB.

### 2.3 Heutige Abdeckung

- **Bein B6**, Auto-Gain-Abschnitt `DspGoldenTestMain.cpp:2433-2621`: flache Kurve, 0-dB-Bell,
  +6-dB-Shelf über dem Gitter, identisches Mid/Side-Paar, Dynamik, Output-Trim. **Kein Grenzfall,
  keine extreme Kurve.** Die Behauptungszeile `tools/beweise.ps1:723` zählt genau diese Fälle auf
  („Auto-Gain ergibt bei flacher Kurve exakt 0 dB und bei einem +6-dB-Shelf ueber dem Gitter
  -6 dB, ein identisches Mid/Side-Paar faellt exakt auf den Stereo-Fall zusammen, und die
  Dynamik bewegt ihn nicht") — eine Aussage über Ränder fehlt.
- **Bein B3c** `EqCopSchemaTest` (`tools/beweise.ps1:702`) prüft `dsp_bericht` nur gegen
  **Fixtures** (`SchemaTestMain.cpp:1257-1300`: `state-report-mit-dsp.json`, `state_report.json`,
  `state-report-dsp-hash-passt-nicht.json`) und gegen den Rückbau auf Fassung 4. Es prüft nie
  eine **selbst gebaute** `baueBericht`-Ausgabe gegen `$defs/dsp_bericht`.
- **Register:** keine Zeile zur Klemmung.
- **Absichtskommentar, der ausdrücklich erlaubt:** keiner; `DspProgramm.cpp:91-95` begründet nur
  den Kurzschluss bei flacher Kurve.

### 2.4 Urteil

**BESTÄTIGT**, mit der Präzisierung der Gegenprüfung: der Befund zerfällt in zwei getrennt
reparierbare Teile.

- **(a) Vertrag und Herleitung.** Die Grenze ±120 dB steht mit einer falschen Begründung in
  Schema und Entscheid, und `baueBericht` kann sie überschreiten. Latent, kein Produktsender.
  Das ist Begründungs- und Nachweisarbeit → **HÄRTUNG**, low.
- **(b) Audiofolge.** Mit vertragsgültigen Werten hebt eingeschalteter Auto-Gain um bis zu
  +43 dB an, und weil das Gitter erst bei 20 Hz beginnt (`DspProgramm.h:72-77`,
  `DspProgramm.cpp:76-83`), trifft diese Anhebung ungemessenen Gleichanteil und Infraschall voll.
  Eine Klemmung auf ±120 dB (linear 1e6) löst das nicht. → **LÜCKE**.

**Kategorievorschlag insgesamt: LÜCKE, Schwere low** — wie in `BEFUNDE.md:635`. Teil b steigt
auf **medium**, wenn der User einen Deckel bejaht, denn dann ist heutiges Verhalten eine
verfehlte Zusage und nicht nur eine fehlende Regel. Klasse K3.

### 2.5 Kleinster Fix und Ort des Rotbeweises

**Fix (a), ohne Produktantwort baubar.** Den `$comment` in
`eq-ipc-v3.schema.json:2325` und den Begründungstext von E2-7 (`SONDE-015.md:2951`) auf das
tatsächlich Erreichbare berichtigen und `baueBericht` (`NakamaTransaktion.cpp:796`) auf die
Vertragsgrenze klemmen, statt sie stillschweigend zu reißen. Die Klemmung gehört sichtbar
gemeldet — `dsp_klemmung` (`eq-ipc-v3.schema.json:2300-2313`) trägt bereits die Form
`id`/`gemeldet`/`wirksam`; ihre Begrenzung auf höchstens eine Klemmung je Slot (E2-7) muss dafür
um einen globalen Eintrag erweitert werden, und das ist eine Vertragsänderung, die Reader,
Writer und Fixtures gemeinsam anfassen muss (`BEFUNDE.md:811` sagt das ausdrücklich).
Solange der Sender nicht angeschlossen ist, genügt Klemmen plus korrigierte Herleitung.

**Fix (b), erst nach Produktantwort.** Den angewandten Ausgleich in `leiteAutoGainAb`
(`DspProgramm.cpp:132-135`) vor der Übergabe an `autoGainLin` (`:266-267`) auf einen
Produktdeckel begrenzen und den Deckel ehrlich melden. Der Ort ist der Worker, nicht der
Audiothread — echtzeitfest, keine Allokation, ein Vergleich.

**Rotbeweis.**
- Teil a: Bein **B3c** `EqCopSchemaTest` (`tools/beweise.ps1:702`), neuer Fall in
  `SchemaTestMain.cpp` neben `:1257-1300`: acht Low-Shelves +12 dB Q 8 bei 1 kHz durch
  `baueBericht`, Ausgabe gegen `$defs/dsp_bericht` validieren. **Heute rot**
  (−199,77 dB gegen `minimum: -120`). Gegenfall: acht Shelves Q 0,707 (−92,29 dB) bleibt grün.
- Teil b: Bein **B6**, neuer Fall im Auto-Gain-Abschnitt neben
  `DspGoldenTestMain.cpp:2433-2621`: acht High-Cuts 20 Hz Q 0,15, Auto-Gain an,
  Gleichanteil plus 10-Hz-Ton am Eingang; Erfolgskriterium ist der beschlossene Deckel.
  **Heute rot** (+150 dB beziehungsweise +43 dB im vertragskonformen Gegenfall).
  Die Zusage trägt danach die Behauptungszeile `tools/beweise.ps1:723`.

### 2.6 Trennung Technik / Produkt

**Ohne Antwort baubar:** Teil a vollständig — berichtigte Herleitung, Klemmung im Bericht,
Meldeform, Schema- und Fixture-Pflege, der B3c-Fall.

**Hängt an einem Produktentscheid:** Teil b — **ob** der automatische Ausgleich eine Obergrenze
bekommt und ungefähr welche Größenordnung. **Keine der Karten U44 bis U47 trifft das:**
U44 ist der Sprungübergang, U45 der Threshold-Pegelbegriff, U46 Decramping, U47 die Samplerate.
**Es braucht eine neue Karte** (nächste freie Kennung wäre U54; höchste vergebene ist U53).

**Wirkung in Musikersprache und Empfehlung:** Der AUTO-Schalter neben OUTPUT rechnet aus, wie viel
lauter oder leiser deine EQ-Kurve das Signal macht, und zieht das am Ausgang wieder ab. Wenn eine
Einstellung sehr viel wegschneidet — mehrere steile Tiefpässe übereinander, wie sie beim Suchen
oder in einem geladenen Preset vorkommen —, kommt die Rechnung auf „hier fehlt fast alles" und
dreht entsprechend weit auf; heute ohne jede Obergrenze. Zu hören ist dann nicht die Musik,
sondern das, was von der Rechnung gar nicht erfasst wird: tieffrequentes Rumpeln unterhalb von
20 Hz und Gleichanteil, massiv verstärkt. Das kann Boxen und Kopfhörer wirklich gefährden.
**Empfehlung: Deckel.** Option 1 (empfohlen): Der Ausgleich geht höchstens bis zu einem festen
Betrag, und Nakama sagt ehrlich, dass es gedeckelt hat — bei allen normalen Einstellungen ändert
sich dadurch gar nichts. Option 2: kein Deckel wie heute, volle Kompensation auch in absurden
Fällen. Die Höhe des Deckels ist Technik und bleibt beim Dirigenten.

---

## 3. T3-16-04 — Fehlende Mono-Regel des Auto-Gain (W08)

*Befundzeile: `BEFUNDE.md:723` · Feldzeile: `BEFUNDE.md:754` und `roh/phase-16-abdeckung.md:15` (F07) ·
Rohbericht: `roh/phase-16-d-dsp.md:30-51` (D-01) · Arbeitsübersicht: `BEFUNDE.md:811`*

### 3.1 Quellkette, zitiert

**Station 1 — Mono ist ein zugelassenes Layout.**
`eq-copilot/plugin/sonde/SondeProcessor.cpp:311-322` (`isBusesLayoutSupported`, Kommentar
NAK-283 F04): Eingang gleich Ausgang UND Mono oder Stereo.

**Station 2 — die Kanalzahl erreicht den Programmbau nicht.**
`SondeProcessor::prepareToPlay` (`:232-269`) legt die Kanalzahl nur für die **Analyse** ab
(`:245`, `v3Channels.store (getTotalNumInputChannels())`) und gibt dem Transaktionskern
ausschließlich die Samplerate (`:259`, `transaktion->setzeSamplerate (sichereRate)`).
`baueProgramm` (`eq-copilot/plugin/dsp/DspProgramm.cpp:162-164`) nimmt
`satz`, `samplerate`, `generation` — **keine Kanalzahl**. `baueBericht`
(`NakamaTransaktion.cpp:791-796`) baut das Programm ebenso nur aus `tk.samplerate()`.

**Station 3 — die Ableitung rechnet immer zwei Ausgangsseiten.**
`DspProgramm.cpp:104-129`:

```
        std::complex<double> hStereo { 1.0, 0.0 }, hLinks { 1.0, 0.0 }, hRechts { 1.0, 0.0 };
        std::complex<double> hMid    { 1.0, 0.0 }, hSide  { 1.0, 0.0 };
        ...
        const auto diagonal = (hMid + hSide) * 0.5;
        const auto hL = hStereo * hLinks  * diagonal;
        const auto hR = hStereo * hRechts * diagonal;

        summe += 0.5 * (std::norm (hL) + std::norm (hR));
```

**Station 4 — im Monobus wird nur L ausgegeben.**
`eq-copilot/plugin/dsp/DspKern.cpp:891-895` dupliziert den Eingang:

```
        dryL[i] = (double) kanaele[0][i];
        dryR[i] = (numKanaele > 1) ? (double) kanaele[1][i] : dryL[i];
```

`:508-513` löst die Kanalkomponente heraus, `:594-601` schreibt sie zurück; ein `right`-Band
berührt nur `R[i]` (`:510`, `:598`), ein `side`-Band hört bei L = R die Null (`:512`).
`:720` multipliziert den aus **beiden gedachten** Kanälen abgeleiteten Auto-Gain auf beide Puffer.
`:1077-1085` schreibt bei Mono nur Kanal 0 hinaus:

```
        float fl = (float) l;
        if (! std::isfinite (fl) && std::isfinite (l)) { fl = 0.0f; zaehlerEingaenge.fetch_add (1, ...); }
        kanaele[0][i] = fl;
        if (numKanaele > 1)
        {
```

**Gemessene Größenordnung (Phase 16, echter Kern, hier nicht erneut ausgeführt):**
48 kHz, Mono, ein Low-Shelf 20 kHz / +12 dB / Q = 1/√2 im Modus `right`, 250-Hz-Sinus.
Ohne Auto-Gain: rund 0 dB Änderung. Mit Auto-Gain: **−9,177564 dB**; `side`: **−7,839905 dB**
(`roh/phase-16-d-dsp.md:23`, `:34`). Gegenfälle im selben Lauf: `left` +2,822436 dB,
`mid` +4,160095 dB, `stereo` +0,082819 dB (`:45`).

### 3.2 Die Zusage

**Keine Mono-Fachregel gefunden** — aber eine **datierte Produktabsicht**, die den Fall
entscheidet. `design/abnahmen/2026-08-24-auto-gain-schalter.md`, Abschnitt 3 („Wirkung an"):

> „Nakama rechnet den Pegelgewinn der aktuellen Kurve und zieht ihn am Ausgang ab. Der angezeigte
> Output-Wert wandert dabei sichtbar mit — ein unsichtbar wirkender Ausgleich wäre ein stiller
> Eingriff."

Und die gewählte Option des Users im selben Dokument, wörtlich:

> „Aus = alles bleibt wie heute, du drehst selbst. An = Nakama gleicht aus."

Dazu R4 (`docs/beweise/SONDE-015.md:1603-1616`) und seine Feinheit 3 (`:1622`), die die
Zweikanalformel **ausdrücklich als Stereo-/M-S-Näherung** kennzeichnet:

> „Die je Seite wirksame Übertragung ist `H_L(f) = H_stereo(f) · H_left(f) · (H_M(f) + H_S(f))/2`
> und `H_R(f) = H_stereo(f) · H_right(f) · (H_M(f) + H_S(f))/2` … danach
> `E(f) = ½ · (\|H_L(f)\|² + \|H_R(f)\|²)`."

Im Monobus gibt es keine zweite Seite. Der abgezogene Betrag ist dort **nicht** der Pegelgewinn
der Kurve, die am Ausgang wirkt.

### 3.3 Heutige Abdeckung

- **Bein B6**, Auto-Gain-Abschnitt `DspGoldenTestMain.cpp:2433-2621`: ausschließlich Stereo.
  Kein Mono-Adapter, keine Kanalzahlvariante.
- **Bein A16** `EqCopProbeeqNullTest` (`tools/beweise.ps1:688`) ist das einzige Bein mit
  Monoaussage; seine Behauptung lautet „Mono und Stereo mit gleichem Ein- und Ausgang werden
  angenommen (im Monobus bleibt der Passthrough bitgleich, und das Band rechnet endlich)".
  Passthrough und Endlichkeit — **nicht** der Ausgleichsbetrag. Ein neutrales Programm hat
  Auto-Gain 0 dB und kann den Fehler deshalb prinzipiell nicht zeigen.
- **Register:** NAK-283 F04 regelt die Layoutzulassung, nicht die Kurvenableitung.
- **Absichtskommentar, der ausdrücklich erlaubt:** keiner. `DspProgramm.cpp:121-124` begründet
  nur den Diagonalterm gegenüber dem Kreuzterm.

### 3.4 Urteil

**BESTÄTIGT**, präzisiert: die Gegenprüfung hat die Defektlesart mit dem Argument abgewehrt,
die Stereoformel sei ausdrücklich entschieden und korrekt ausgeführt. Das hält für die
**Formel**. Es hält **nicht** für die Frage, ob sie auf ein Layout angewandt werden darf, für
das sie nie entschieden wurde. Die Abnahme vom 24.08.2026 benennt die Produktabsicht so eng,
dass die zulässigen Mono-Regeln auf eine zusammenschrumpfen: ausgeglichen wird, was am Ausgang
wirklich passiert.

**Kategorievorschlag: LÜCKE** (fehlende Anpassung an einen gebauten Eingang, keine gebrochene
Formelzusage), **Schwere medium** — hörbare 7,8 bis 9,2 dB Absenkung eines sonst neutralen
Wegs, erreichbar über geladenen Zustand plus den Host-Schalter `v2.global.auto_gain`.
Klassen K3 und K4.

### 3.5 Kleinster Fix und Ort des Rotbeweises

**Fix.** Zwei eng gekapselte Schritte.

1. **Kanalzahl in den Programmbau geben.** `SondeProcessor::prepareToPlay` (`:252-264`) reicht
   neben `setzeSamplerate` (`:259`) auch die Kanalzahl an den Transaktionskern; `baueProgramm`
   (`DspProgramm.cpp:162-164`) nimmt sie entgegen, `baueBericht` (`NakamaTransaktion.cpp:791-796`)
   baut sein Programm mit derselben Zahl. Ein Layoutwechsel löst in JUCE ohnehin
   `prepareToPlay` aus, damit ist der Neubau des Programms und des abgeleiteten Berichts an
   dieselbe Naht gebunden (Regel „Beziehungen mitprüfen": Layout setzen ↔ Programm erneuern).
2. **Für Mono den tatsächlich ausgegebenen Weg auswerten.** Nicht `right`/`side` pauschal
   streichen — die Gegenprüfung warnt zu Recht, dass eine spätere `mid`/`side`-Stufe den
   virtuellen rechten Kanal wieder nach links mischt (`roh/phase-16-d-dsp.md:45`). Stattdessen in
   `leiteAutoGainAb` (`DspProgramm.cpp:85-136`) einen Mono-Zweig führen, der je Gitterstelle die
   Bänder **in Slotreihenfolge** als 2×2-Übertragungsmatrix auf den Eingangsvektor (1, 1) anwendet
   und nur die L-Komponente ausliest: `E(f) = |H_LL(f) + H_LR(f)|²`. Das ist für den Ruhezustand
   exakt derselbe Weg, den der Kern sample-weise fährt, kostet 121 Gitterstellen × höchstens acht
   2×2-Produkte im **Worker** und lässt den Stereozweig unverändert — M-37 und die
   Mid/Side-Goldens bleiben bitgenau, weil sie den Stereozweig treffen.

**Rotbeweis.** Bein **B6**, neuer Mono-Abschnitt neben `DspGoldenTestMain.cpp:2433-2621`:
48 kHz, Mono, ein Low-Shelf 20 kHz / +12 dB / Q = 1/√2 im Modus `right`, 250-Hz-Sinus;
Erfolgskriterium: RMS mit Auto-Gain an weicht vom Lauf ohne Auto-Gain um ≤ 1e−6 ab.
**Heute rot** (−9,177564 dB). Zweiter Fall `side` (heute −7,839905 dB). Regressionsfälle im
selben Abschnitt: `right` → `mid`-Kaskade (der rechte Weg **muss** dort wieder eingehen),
Layoutwechsel Mono ↔ Stereo mit erneuertem Bericht, und die bestehenden Stereo-/M-S-Goldens
müssen grün bleiben. Gegenwache: Bein **A16** (`tools/beweise.ps1:688`), dessen
Monoaussage um „und der Ausgleich rechnet nur mit dem ausgegebenen Kanal" zu ergänzen ist.
Die Behauptungszeile `tools/beweise.ps1:723` trägt die Zusage.

### 3.6 Trennung Technik / Produkt

**Kein Produktentscheid nötig.** Die Regel ist durch die datierte Abnahme vom 24.08.2026
(„Pegelgewinn der aktuellen Kurve … am Ausgang ab") und durch R4s ausdrückliche Kennzeichnung
der Zweikanalformel als **Stereo**-Näherung bereits festgelegt. Was fehlt, ist die Umsetzung
auf ein Layout, das nach der Abnahme zugelassen wurde — das ist Technik. Keine der Karten
U44 bis U47 betrifft ihn, und es braucht auch keine neue.

**Ohne Antwort baubar:** alles unter 3.5 — Kanalzahl-Verdrahtung, Mono-Zweig der Ableitung,
Neubau bei Layoutwechsel, die Goldens.

**Zwei Randpunkte, die der Dirigent datiert nach `docs/offene-punkte.md` legen sollte, nicht als
Karte:**
- Ein `right`- oder `side`-Band ist im Monobus wirkungslos (der Kern schreibt Kanal 1 nicht,
  und bei L = R ist Side null). Nach dem Gesetz „keine toten UI-Elemente" gehört das später
  ehrlich angezeigt. Die Fläche ist geparkt; der Punkt gehört registriert, nicht jetzt gebaut.
- Persistente Kanalwünsche dürfen beim Layoutwechsel nicht verworfen werden
  (`BEFUNDE.md:811`, „persistente Kanalwünsche bewahren") — dieselbe Regel wie bei der
  Nyquist-Kappung M-12: geklemmt wird beim Bauen, gespeichert bleibt der Userwunsch.

---

## 4. T3-15-06 — Threshold-Pegelbegriff des dynamischen EQ (W35)

*Befundzeile: `BEFUNDE.md:632` · Gegenprüfung: `roh/phase-15-gegenpruefung-paket2.md:104-162` (K2) ·
Finder: `roh/phase-15-paket2-dsp-numerik.md:180-208` (P15-D-02) · Herabstufung: `BEFUNDE.md:655` ·
Arbeitsübersicht: `BEFUNDE.md:838`*

### 4.1 Quellkette, zitiert

**Station 1 — die Hüllkurve vergleicht gegen die Momentanleistung und lädt den Hold neu.**
`eq-copilot/plugin/dsp/DspFilter.h:312-318`:

```
    double tick (const HuellkurveKoeffizienten& k, double leistungEin) noexcept
    {
        if (leistungEin > leistung)
        {
            leistung = k.attackPol * leistung + (1.0 - k.attackPol) * leistungEin;
            holdRest = k.holdSamples;
        }
```

Der Kommentar `:305-311` nennt genau diese Reihenfolge als M-26-Zusage, `:258-265` begründet
die Rechnung auf der Momentanleistung („das waere kein RMS" bezieht sich auf die Alternative,
den Betrag zu mitteln).

**Station 2 — die Kennlinie liest genau diesen Wert.** `eq-copilot/plugin/dsp/DspKern.cpp:571-573`:

```
                if (detektorAktiv)
                    gDyn = dynamischeKennlinie (leistungInDb (z.huelle.leistung), thresholdDb, rangeDb);
```

`dynamischeKennlinie` steht bei `DspFilter.h:355-361` und liefert unter Threshold bitgenau 0,0
(`if (! (ueber > 0.0)) return 0.0;`, `:358`) — **M-19 ist nicht verletzt**, die Gegenprüfung hat
die Defektlesart hier ausdrücklich widerlegt. Die gemeinsame Leistung beider Komponenten
entsteht bei `DspKern.cpp:539-542` (E-5).

**Gemessene Größenordnung (Altbasis, unabhängig nachgerechnet):** 1 kHz, Bandpass Q 0,707, L = R:
Vertragsdefault 10/0/100 ms **+2,12 dB**; Hold ≥ halbe Periode **+3,01 dB** (der theoretische
Spitzen-/Effektivwertabstand); Rauschen 10/30/100 ms **+8,66 dB**; Quadraturton in jeder
Einstellung **+0,00 dB** (`roh/phase-15-gegenpruefung-paket2.md:147-153`).

### 4.2 Die Zusage

**Keine Zusage zum Pegelbezug des Thresholds.** Was es gibt, benennt die **Familie**, nicht den
Bezug zum Signalpegel — R7 Feinheit 3, `docs/beweise/SONDE-015.md:1711`:

> „RBJ-Bandpass mit konstanter Spitzenverstärkung auf `freq_hz` und `q` desselben Slots,
> angewandt auf das **Eingangssignal des Bandes**, danach quadratischer Mittelwert über eine
> Ein-Pol-Hüllkurve mit den Koeffizienten `exp(−1/(fs · τ))` und getrenntem Hold-Zähler."

Und M-18 (`SONDE-015.md:1061`) definiert den Pegel **selbstbezüglich**:

> „`g_dyn = dynamic_range_db · min(1, max(0, e_db − threshold_db) / 12 dB)`, wobei `e_db` der
> RMS-Pegel des Detektors nach Attack/Hold/Release ist."

Der Bau entspricht beiden wörtlich. Was fehlt, ist die Aussage, **gegen welchen Pegelbegriff des
Materials** der eingestellte `threshold_db` wirkt.

### 4.3 Heutige Abdeckung

- **Bein B6**, Knieabschnitt `DspGoldenTestMain.cpp:1691-1728` und Sprungantwort
  `:1894-1970` / `:1972-2059`: **alle** Messungen fahren einen Stereoton auf der Bandmitte
  (L sin, R cos). Der Kommentar `:1699-1703` sagt es selbst: „Ein Stereoton auf der Bandmitte
  (L sin, R cos) hält die Detektorleistung konstant auf a^2/2". Genau in diesem Signal ist der
  Versatz **konstruktionsbedingt exakt null**. Der Beweisschatten ist damit im Test verankert und
  in E-25 (`SONDE-015.md:3419`) ausdrücklich begründet.
- **Behauptungszeile** `tools/beweise.ps1:723` nennt Knie, Reihenfolge und Zeiten, nie eine
  Aussage über schwankende Leistung.
- **Kein Verbraucher des Pegels** auf dem Draht: die v3-Flatbuffers tragen
  `band_dynamic_gain_db` (die Auslenkung), kein Pegelfeld.
- **Register:** keine Zeile.
- **Absichtskommentar, der ausdrücklich erlaubt:** keiner.

### 4.4 Urteil

**BESTÄTIGT** in der Fassung nach der Gegenprüfung: keine Zusage verletzt, M-19 hält, der Bau
ist wörtlich R7 Feinheit 3 und M-26. Übrig bleibt die fehlende Regel plus ein echter
Beweisschatten.

**Kategorievorschlag: LÜCKE, Schwere low.** Klassen K3 und K5.

### 4.5 Kleinster Fix und Ort des Rotbeweises

**Fix, Variante RMS (U45 Weg 1).** Die Kennlinie nicht mehr direkt aus dem Attack-/Hold-/
Release-Zustand speisen, sondern aus einem getrennten, **symmetrischen** Leistungsmittel mit
fester Fensterlänge; Attack, Hold und Release formen weiterhin, **wie schnell der Gain folgt**,
nicht mehr, **ab wann** er einsetzt. Ort: `DspFilter.h:290-329` (ein zweiter, schlanker
Ein-Pol-Zustand neben `HuellkurveZustand`) und `DspKern.cpp:571-573` (die Kennlinie liest den
neuen Wert). Kosten im Audiothread: ein zusätzlicher Pol je Sample und Band, keine Allokation,
kein `log10` (das bleibt am Steuerschritt).

**Fix, Variante spitzennah (U45 Weg 2).** Kein Code. R7 Feinheit 3 und M-18 um den Satz ergänzen,
dass der Threshold auf den spitzennahen Detektorpegel wirkt, und die Fensterwirkung von Hold
ausdrücklich benennen.

**In beiden Fällen der gleiche Rotbeweis.** Bein **B6**, neuer Fall neben
`DspGoldenTestMain.cpp:1691-1728`: L = R-Sinus konstanten RMS-Pegels und ein rauschartiges
Signal gleicher Leistung, beide gegen den Quadraturton gleicher Leistung, mit den
Vertragsdefaults 10/0/100 ms. Heute messen dieselben Einstellungen eine Auslenkung von
**−6,09 gegen −4,50 dB**. Erfolgskriterium: Differenz ≤ 0,1 dB (Variante RMS) beziehungsweise
die neu gefasste Zusage mit ihrer dokumentierten Spanne (Variante spitzennah).
Behauptungszeile `tools/beweise.ps1:723`.

### 4.6 Trennung Technik / Produkt

**Ohne Antwort baubar:** nichts Wesentliches — die Fensterlänge, die Wahl des Pols und die
Testform sind Technik, aber sie hängen alle an der Richtung.

**Hängt an einem Produktentscheid:** ja. **Karte U45 trifft den Befund** genau
(„Dynamischer EQ: soll die Schwelle auf Spitzen oder auf die Durchschnittslautstärke reagieren?",
Status `offen`, seit 18.09.). Ihr Satz „Attack, Hold und Release bleiben in beiden Fällen so
einstellbar wie heute" ist mit dem Fix oben vereinbar und beschreibt die technische Folge korrekt:
bei Weg 1 trennt sich das Messfenster von der Ballistik.

**Wirkung in Musikersprache und Empfehlung** (die Karte formuliert es bereits so; hier nur zur
Bestätigung): Bei Weg 1 (RMS, empfohlen) greift ein dynamisches Band nach der durchschnittlichen
Lautstärke in seinem Frequenzbereich — es arbeitet ruhiger und ignoriert einzelne Schläge.
Bei Weg 2 (Peak) reagiert dasselbe Band auf jeden Snare-Schlag sofort und kann bei perkussivem
Material flattern. Heute liegt Nakama je nach Hold- und Attack-Einstellung mal beim einen, mal
beim anderen: derselbe Schwellwert greift bei einem Klavierton rund 3 dB und bei Rauschen bis
8,7 dB früher, als eine Durchschnittslesart erwarten lässt. **Empfehlung: Weg 1.**

---

## 5. T3-15-10 — Decramping-Prüfauftrag ohne Entscheid (W35)

*Befundzeile: `BEFUNDE.md:636` · Gegenprüfung: `roh/phase-15-gegenpruefung-paket2.md:321-360` (K6) ·
Finder: `roh/phase-15-paket2-dsp-numerik.md:270-282` (P15-D-06) · Herabstufung: `BEFUNDE.md:656` ·
Arbeitsübersicht: `BEFUNDE.md:838`*

### 5.1 Quellkette, zitiert

**Station 1 — der Satz steht in einem bindenden Dokument.**
`docs/FL-Nakama-Sonden-Design-Entwurf.md:2980-2981`:

```
- minimumphasige RBJ-Biquads als Referenz für statische Bell-, Shelf-, Notch- und Cut-Filter;
  nahe Nyquist wird ein matched-analog-/Orfanidis-artiges Decramping geprüft;
```

Derselbe Satz ist in `docs/beweise/SONDE-015.md:210-211` ausdrücklich unter den **bindenden**
Sätzen zitiert.

**Station 2 — gebaut ist reines RBJ.** `eq-copilot/plugin/dsp/DspFilter.h:116-131`
(`entwurfBell`, die klassische RBJ-Form mit `w0 = 2π f / fs`, `alpha = sin(w0)/(2q)`),
und die Frequenz wird lediglich auf `0,45 · fs` gekappt (`:106-114`, `kappeNyquist`).
Kein Vorverzerrungs- oder Anpassungsschritt.

**Station 3 — die Repo-Suche am HEAD.** `grep -rni "decramping|orfanidis|cramping|matched-analog"`
über `docs/`, `design/`, `eq-copilot/`, `wissen/` findet außerhalb der Auditakte und der
JUCE-Abhängigkeit nur: `docs/FL-Nakama-Sonden-Design-Entwurf.md:2981`,
`docs/beweise/SONDE-015.md:211`, das Archiv `docs/archiv/FL-Nakama-Sonden-Design-Entwurf-0.4-2026-08-20.md:2704`,
die Registerzeile NAK-311 selbst (`docs/offene-punkte.md:316`), Karte U46 in
`docs/plan/fragen.json:78-79` und die daraus abgeleiteten Plantexte.
**Keine M-Zeile, kein E-/R-Entscheid, keine eigene Registerzeile, kein Planschritt.**

**Gemessene Größenordnung (Altbasis, unabhängig bestätigt):** Bell 16 kHz Q 2 +12 dB, gemessen
bei 19,2 kHz, weicht vom analogen Prototyp ab um −6,48 dB (44,1 kHz), −5,52 dB (48 kHz),
−1,19 dB (96 kHz); bei 16 kHz selbst ist die Abweichung exakt 0 (Vorverzerrung auf f0).
Derselbe gespeicherte Zustand klingt bei 44,1 gegen 96 kHz im Bereich 12 bis 20 kHz um bis zu
rund 5 dB verschieden (`roh/phase-15-gegenpruefung-paket2.md:352-358`).

### 5.2 Die Zusage

**Keine verletzte Zusage.** Im Gegenteil: die geltenden Zusagen fordern **RBJ** als Referenz und
würden durch ein eingebautes Decramping **gebrochen**.
`docs/beweise/SONDE-015.md:1048` (M-10):

> „Die gemessene Betragsantwort des **Audiopfads** liegt typisch innerhalb ±0,05 dB und an den
> Extrempunkten innerhalb ±0,1 dB an der analytischen RBJ-Antwort."

M-11 (`:1049`) dehnt das auf alle sechs Typen aus, M-13 (`:1051`) verlangt eine im Test
eigenständig ausgeschriebene RBJ-Formel. Der Entwurfssatz sagt „wird **geprüft**", nicht
„wird gebaut" — er fordert kein Produktverhalten.

**Offen ist ein Prüfauftrag in einem bindenden Dokument ohne Aktenzeichen.** CLAUDE.md verlangt
für solche Widersprüche eine Zeile in `docs/offene-punkte.md`; eine eigene Zeile gibt es nicht,
nur die Sammelzeile NAK-311.

### 5.3 Heutige Abdeckung

- **Bein B6**, C-Abschnitt `DspGoldenTestMain.cpp:1121-1177`. Die Referenz ist `refEntwurf`,
  also die im Test ausgeschriebene **RBJ**-Formel (`:1152`) — der Test kann die Abweichung vom
  **analogen** Prototyp prinzipiell nicht sehen. Zudem ist die höchste Prüffrequenz 8 kHz
  (`:1122-1131`: Bell 1 kHz, Bell 1 kHz Q 12, Low-Shelf 200 Hz, High-Shelf 5 kHz, Notch 1 kHz,
  Low-Cut 100 Hz, High-Cut 8 kHz). Die oberste Oktave ist unbeprobt.
- **Behauptungszeile** `tools/beweise.ps1:723` sagt ausdrücklich „an der analytischen Antwort" —
  sie zementiert RBJ als Referenz.
- **Register:** keine Zeile.

### 5.4 Urteil

**BESTÄTIGT als Härtung.** Kein Produktfehler, keine fehlende erreichbare Regel — die Regel
existiert und ist gemessen. Was fehlt, ist der Abschluss eines Prüfauftrags, der in einem
bindenden Dokument steht.

**Kategorievorschlag: HÄRTUNG, Schwere low.** Klassen K3 und K5.

### 5.5 Kleinster Fix und Ort des Rotbeweises

**Fix bei Antwort „verwerfen" oder „vertagen" (der wahrscheinliche Fall).** Eine datierte
Entscheidung plus Registerzeile in `docs/offene-punkte.md`, Wortlaut etwa „Decramping geprüft
und vertagt, RBJ bleibt Referenz — R15, M-10", und im Entwurf `:2981` der Satz so gefasst, dass
er den Stand abbildet statt einen offenen Auftrag zu behaupten. **Kein Codebeweis, kein
Rotbeweis** — dafür ein Lauf von `tools/plan/dokuriegel.py` über die geänderten Plandokumente.

**Fix bei Antwort „einführen" — und das ist deutlich teurer, als Karte U46 heute sagt.** Dann
müssten R15 (`SONDE-015.md:2085-2105`) und M-10 bis M-13 (`:1048-1051`) **neu entschieden**
werden, weil die heutige Zertifizierung gegen RBJ läuft; die Behauptungszeile
`tools/beweise.ps1:723` und der ganze C-Abschnitt `DspGoldenTestMain.cpp:1121-1177` samt
Referenzformel wären betroffen. Rotbeweis wäre dann ein neuer B6-Fall: Bell 16 kHz Q 2 +12 dB
bei 44,1 / 48 / 96 kHz gegen den analogen Prototyp mit festgelegter Toleranz — **heute rot**
(−6,48 / −5,52 / −1,19 dB).

### 5.6 Trennung Technik / Produkt

**Ohne Antwort baubar:** nichts. Bei „vertagen" oder „verwerfen" fällt überhaupt keine
Codearbeit an.

**Hängt an einem Produktentscheid:** ja. **Karte U46 trifft den Befund**
(„Höhen-Korrektur des digitalen EQ (Decramping): einbauen, verwerfen oder vertagen?",
Status `offen`). Ihr Empfehlungstext („vertagen") deckt sich mit meiner fachlichen Einschätzung.

**Was der Karte fehlt und der Dirigent vor dem Stellen ergänzen sollte:** Weg 2 („Jetzt einbauen,
im Audio-Kern-Ticket") ist erheblich größer, als der Text vermuten lässt — er zieht eine
Neuentscheidung der bestehenden Filterzusagen R15 und M-10 bis M-13 nach sich, also den Umbau
der gemessenen Referenz, gegen die heute jeder Filtergolden läuft. Das gehört in einer Zeile in
den Kartentext, damit der User nicht aus Versehen den teuersten Weg wählt.

**Wirkung in Musikersprache und Empfehlung:** Eine Glocke, die du ganz oben bei 16 kHz setzt,
klingt heute in einem 44,1-kHz-Projekt anders als dasselbe Preset in einem 96-kHz-Projekt —
oben herum bis zu rund 5 dB Unterschied, und schmaler und schiefer, als ein analoger EQ es täte.
Das ist bei digitalen EQs üblich und betrifft nur die oberste Oktave. **Empfehlung: vertagen.**
Erst den Kern fertig machen; später ein eigener Schritt mit Hörprobe, bei dem sich zeigen muss,
ob der Unterschied überhaupt auffällt. Am Klang ändert sich durch das Vertagen nichts.

---

## 6. T3-15-11 — Falsche Begründung der Steuerrate (W35)

*Befundzeile: `BEFUNDE.md:637` · Gegenprüfung: `roh/phase-15-gegenpruefung-paket2.md:362-398` (K7) ·
Finder: `roh/phase-15-paket2-dsp-numerik.md:283-297` (P15-D-07) · Herabstufung: `BEFUNDE.md:657` ·
Arbeitsübersicht: `BEFUNDE.md:838`*

### 6.1 Quellkette, zitiert

**Station 1 — der falsche Satz im Code.** `eq-copilot/plugin/dsp/DspProgramm.h:64-70`:

```
/** Steuerrate des dynamischen Bandes in Samples: alle so viele Samples
    werden Detektorpegel, Kennlinie und SVF-Koeffizienten neu gerechnet,
    dazwischen laeuft der Mischfaktor linear weiter. Bei 48 kHz sind das
    0,167 ms - feiner als die kuerzeste Attack (0,1 ms). Der DETEKTOR
    selbst laeuft mit voller Audiorate; nur `log10` und `pow` haengen an
    diesem Schritt. */
inline constexpr int kDynamikSchritt = 8;
```

**Station 2 — derselbe Satz im Entscheid.** `docs/beweise/SONDE-015.md:3397` (E-6):

> „8 Samples sind 0,167 ms bei 48 kHz und damit feiner als die kürzeste Attack (0,1 ms); die
> lineare Überblendung dazwischen hält den Gain sample-genau stetig statt in Stufen zu springen."

**Station 3 — die Mechanik.** `eq-copilot/plugin/dsp/DspKern.cpp:557-581`: am Steuerschritt
werden `svfVon = svfNach` gesetzt und `svfNach` neu entworfen (`:574-575`); danach gilt
`const double tSchritt = 1.0 - (double) z.schrittRest / (double) kDynamikSchritt;` (`:579`).
Bei `schrittRest == kDynamikSchritt` ist `tSchritt == 0` — der frisch entworfene Satz wirkt am
Entwurfssample mit Gewicht **null** und erst acht Samples später voll.

**Station 4 — der Vertragsrand.** `eq-copilot/schemas/state/nakama-parameter-v2.json:245-257`:
`attack_ms` mit `"min": 0.1`, `"max": 500.0`, `"host_parameter": true`.

**Nachgerechnet am HEAD:** 8 / 48000 = 0,16667 ms; 8 / 44100 = 0,18141 ms;
15 / 48000 = 0,3125 ms; 15 / 44100 = 0,34014 ms. **0,167 ms ist gröber als 0,1 ms** — der Satz
im Code und im Entscheid stellt die Ungleichung verkehrt herum dar. Bei 96 und 192 kHz wäre er
richtig.

### 6.2 Die Zusage

**Keine Zusage zur Reaktionszeit des Gains.** M-26 (`SONDE-015.md:1069`) betrifft die drei
Stufen der Hüllkurve, und die Hüllkurve selbst läuft mit voller Audiorate (`DspKern.cpp:531-553`).
E-25 (`SONDE-015.md:3419`) rechnet die Steuerrate ausdrücklich in die Prüftoleranz ein:

> „1 ms deckt die Gruppenlaufzeit des Detektor-Bandpasses (0,23 ms bei 1 kHz, Q 0,707) und die
> Steuerrate (höchstens 0,18 ms)"

Zu berichtigen ist die **Begründung** — in E-6 (`SONDE-015.md:3397`) und in
`eq-copilot/plugin/dsp/DspProgramm.h:66-67`.

### 6.3 Heutige Abdeckung

- **Bein B6**, Sprungantwort `DspGoldenTestMain.cpp:1894-1970` mit Toleranz **1 ms** je Stufe
  (Assertion `:1972-1976`, „soll A 20 H 30 R 50 ms +/- 1 ms"). Ein Verzug von 0,167 bis 0,312 ms
  liegt prinzipiell darunter — die Messung **kann** ihn nicht sehen, und E-25 rechnet ihn sogar
  ausdrücklich in die Toleranz hinein.
- **Der W-2-Fall** `:1972-2059` misst die gemeldete Auslenkung **am Steuerschritt**, nicht die
  zwischen zwei Steuerschritten tatsächlich angewandte Verstärkung.
- **Behauptungszeile** `tools/beweise.ps1:723` enthält keine Zahl zur Reaktionszeit.
- **Register:** keine Zeile.

### 6.4 Urteil

**BESTÄTIGT.** Die Arithmetik ist am HEAD nachgerechnet und falsch; die Herabstufung auf
HÄRTUNG trägt, weil keine Zahlenzusage existiert und die Hüllkurve selbst korrekt arbeitet.

**Kategorievorschlag: HÄRTUNG, Schwere low.** Klasse K5.

### 6.5 Kleinster Fix und Ort des Rotbeweises

**Fix.** Kein Verhalten ändern, sondern die Begründung ehrlich machen und die Lücke schließen,
die sie verdeckt hat:

1. `eq-copilot/plugin/dsp/DspProgramm.h:66-67` und E-6 (`SONDE-015.md:3397`) neu fassen:
   8 Samples sind 0,167 ms bei 48 kHz und 0,181 ms bei 44,1 kHz und damit **gröber** als die
   kürzeste einstellbare Attack von 0,1 ms; die angewandte Verstärkung folgt einem Pegelsprung
   nach 8 bis 15 Samples, also 0,167 bis 0,312 ms bei 48 kHz und 0,181 bis 0,340 ms bei
   44,1 kHz. Der Grund bleibt derselbe und bleibt gut: `log10` und `pow` je Sample und Band
   kosten ein Vielfaches der Filterarbeit.
2. Eine **Reaktionsregel** festlegen, etwa „die angewandte Verstärkung erreicht 90 % ihres Ziels
   spätestens nach `attack_ms + 2 · kDynamikSchritt` Samples", und sie messbar machen.

**Rotbeweis.** Bein **B6**, neuer Fall neben `DspGoldenTestMain.cpp:1972-2059`: 48 kHz,
Bell 1 kHz, Range −12 dB, Threshold −40 dB, Attack 0,1 ms, Einsatz in Steuerphase 0, 3 und 7;
gemessen wird die **angewandte** Verstärkung je Sample aus dem Tap, nicht der gemeldete Wert.
Heute erreicht sie 90 % erst nach 8 bis 15 Samples. Gegenfall 96 kHz (dort trägt der ursprüngliche
Satz). Der Fall ist **kein Rotbeweis gegen das Produkt**, sondern der fehlende Nachweis: er wird
gegen die neu gefasste Regel grün und hält sie fest. Behauptungszeile `tools/beweise.ps1:723`.
Für die Plandokumente `tools/plan/dokuriegel.py`.

### 6.6 Trennung Technik / Produkt

**Kein Produktentscheid nötig.** Es gibt keine Zusage zur Reaktionszeit, das Klangverhalten
ändert sich nicht, und ein Verzug von 0,17 bis 0,34 ms ist praktisch nicht hörbar. Reine
Begründungs- und Nachweisarbeit. Keine Karte, keine der vorhandenen U44 bis U47 betrifft ihn.

**Ein Randpunkt, der datiert nach `docs/offene-punkte.md` gehört, nicht als Karte:** Bei
`attack_ms` = 0,1 ms ist die tatsächlich angewandte Gainflanke rund dreimal langsamer als
eingestellt. Der Einstellbereich unterhalb von etwa 0,3 ms ist damit in der Wirkung nahezu
gleich — nach dem Gesetz „keine toten UI-Elemente" ein ehrlich zu benennender Punkt, aber kein
Fehler und keine Frage an den User, solange der Kern nicht umgebaut wird.

---

## 7. F08 — Niedrigratensupport, Projekte unter 44,1 kHz

*Feldzeile: `roh/phase-16-abdeckung.md:16` · Rohbericht: `roh/phase-16-d-dsp.md:24` und
`:53-65` (D-02, ausdrücklich **nicht** als Befund aufgenommen) · Zusammenfassung: `BEFUNDE.md:755` ·
Arbeitsübersicht: `BEFUNDE.md:811` (Schlusszeile „F08 Niedrigratensupport separat")*

### 7.1 Quellkette, zitiert

**Station 1 — der Prozessor nimmt jede endliche Rate.**
`eq-copilot/plugin/sonde/SondeProcessor.cpp:237-238`:

```
    const double sichereRate = std::isfinite (samplerate) && samplerate > 0.0
                            && samplerate <= 768000.0 ? samplerate : 0.0;
```

`:252-256` bereitet den Kern mit genau dieser Rate vor. Gen hat dieselbe Sanityschranke
(`eq-copilot/plugin/src/prozessor/Hostbruecke.cpp:37`).

**Station 2 — das Auto-Gain-Gitter ist sampleratenunabhängig definiert.**
`eq-copilot/plugin/dsp/DspProgramm.h:72-77` (`kAutoGainStellen = 121`, `kAutoGainVonHz = 20.0`,
`kAutoGainBisHz = 20000.0`) und `DspProgramm.cpp:76-83`. Die Auswertung
`DspProgramm.cpp:99-102`:

```
    for (int i = 0; i < kAutoGainStellen; ++i)
    {
        const double f = autoGainGitterHz (i);
        const double w = 2.0 * kPi * f / p.samplerate;
```

**Keine Nyquistprüfung.** Bei 32 kHz liegen vier der 121 Stellen über Nyquist und gehen mit
periodisch gespiegelter Antwort ein; bei 22,05 kHz sind es elf.

**Station 3 — die Bandfrequenzen dagegen werden korrekt gekappt.**
`DspProgramm.cpp:214-215` ruft `kappeNyquist` (`DspFilter.h:106-114`), Deckel `0,45 · fs`.
Der Widerspruch liegt also **nur** im Integrationsgitter der Kompensation.

**Gemessene Größenordnung (Phase 16, echter Kern):** 32 kHz, Bell 14 kHz, +12 dB, Q 4:
der Kern kompensiert **−0,397113 dB**; über die 117 Punkte unter Nyquist ergäbe sich
**−0,234376 dB**. Bei Q 1 wächst der Unterschied auf 0,506656 dB. Bei 44,1 und 48 kHz sind beide
Auswertungen identisch (`roh/phase-16-d-dsp.md:24`, `:57`).

### 7.2 Die Zusage

**Es gibt eine Zusage, und sie steht auf der Seite „nicht unterstützen".**
R4 Feinheit 1, `docs/beweise/SONDE-015.md:1620`:

> „Es ist samplerateunabhängig definiert; da 20 kHz bei jeder unterstützten Rate unter der
> Nyquistgrenze liegt (kleinste Rate 44,1 kHz), braucht es keine Kappung."

Die weiter gefasste Sanityprüfung in `prepareToPlay` ist **keine** gleichrangige Supportzusage —
die Gegenprüfung hat genau deshalb D-02 nicht als Befund aufgenommen
(`roh/phase-16-d-dsp.md:55`, `:61`).

### 7.3 Heutige Abdeckung

- **Beine B6 und A16** fahren 44,1 / 48 / 96 / 192 kHz (Behauptungszeilen
  `tools/beweise.ps1:723` beziehungsweise `:688`). **Keine Rate unter 44,1 kHz** wird gefahren,
  weder als zugelassener noch als abgewiesener Fall.
- **Nichts weist eine niedrige Rate ab:** `SondeProcessor.cpp:237-238` akzeptiert sie, der
  isolierte JUCE-VST3-Wrapper reicht sie ohne untere Schranke durch
  (`roh/phase-16-d-dsp.md:59`).
- **Register:** keine Zeile; NAK-254 betrifft die Meldung der Bandfrequenzkappung, nicht das
  Integrationsgitter.

### 7.4 Urteil

**BESTÄTIGT als Abdeckungsfrage — kein neuer Befund**, wie `roh/phase-16-abdeckung.md:16` sagt.
**Präzisiert:** die Produktrichtung ist durch R4 Feinheit 1 bereits benannt (kleinste unterstützte
Rate 44,1 kHz). Offen ist nicht, **was richtig ist**, sondern nur, ob Nakama die Grenze künftig
**ehrlich durchsetzt** oder den Support **erweitert**.

**Kategorievorschlag: LÜCKE, Schwere low.** Keine Zusage verletzt (unter 44,1 kHz ist nichts
zugesagt), aber es fehlt die ehrliche Ablehnung: heute verarbeitet der EQ still mit einem
Kompensationsgitter, das bei dieser Rate teilweise Unsinn auswertet. Klasse K3.

### 7.5 Kleinster Fix und Ort des Rotbeweises

**Fix bei Antwort „nicht unterstützen" (U47 Weg 1, empfohlen).** In
`SondeProcessor::prepareToPlay` (`:237-238`) und in Gens `Hostbruecke.cpp:37` die untere Schranke
ergänzen: eine Rate unter 44,1 kHz wird wie `samplerate = 0` behandelt, der Kern bleibt
unvorbereitet, der Pfad bleibt Passthrough, und der Grund wird über den bestehenden
Zustands-/Berichtsweg lesbar gemacht. Kein Audiothread berührt, kein DSP geändert.
**Wichtig für die Erwartung:** die sichtbare Anzeige dafür existiert noch nicht (Fläche geparkt);
„ehrlich melden" heißt heute, dass der Prozessor den Grund über den vorhandenen Getter führt,
die Anzeige folgt mit der Oberfläche.

**Fix bei Antwort „unterstützen" (U47 Weg 2).** Das Gitter muss sampleratenabhängig werden:
Obergrenze `min(20 kHz, 0,45 · fs)`, Mittelung über die verbleibenden Punkte, und R4 Feinheit 1
(`SONDE-015.md:1620`) ist neu zu fassen. Das berührt die bestehenden Auto-Gain-Goldens.

**Rotbeweis.** Bein **A16** `EqCopProbeeqNullTest` (`tools/beweise.ps1:688`), neuer Fall mit
32 kHz und einem belegten Bell 14 kHz +12 dB Q 4 sowie eingeschaltetem Auto-Gain:
heute engagiert sich der Kern und kompensiert −0,397113 dB; nach dem Fix (Weg 1) muss der Ausgang
bitgleich zum Eingang bleiben und der Grund lesbar sein. **Heute rot.** Gegenfall im selben Bein:
44,1 kHz bleibt unverändert grün. Die Zusage trägt die Behauptungszeile `tools/beweise.ps1:688`
und, für das Gitter, `:723`.

### 7.6 Trennung Technik / Produkt

**Ohne Antwort baubar:** nichts — der Fix läuft in zwei gegenläufige Richtungen.

**Hängt an einem Produktentscheid:** ja. **Karte U47 trifft die Frage** genau
(„Projekte mit weniger als 44,1 kHz: unterstützen oder ausschließen?", Status `offen`).
Zwei Präzisierungen für den Dirigenten:
- Der Zusatz „und meldet den Grund ehrlich in der Anzeige" beschreibt eine Fläche, die es noch
  nicht gibt; die Karte sollte ihn so fassen, dass sie keine sofortige Anzeige verspricht.
- Es ist kein Beleg vorhanden, dass eine aktuelle FL-Konfiguration überhaupt 32 kHz anbietet
  (`roh/phase-16-d-dsp.md:55`). Die praktische Dringlichkeit ist deshalb niedrig; die Karte kann
  das sagen, damit der User sie richtig gewichtet.

**Wirkung in Musikersprache und Empfehlung:** In einem Projekt mit sehr niedriger Abtastrate
(22,05 oder 32 kHz — bei FL ungewöhnlich) rechnet Nakamas automatischer Lautstärke-Ausgleich mit
Stützpunkten im Höhenbereich, den es bei dieser Rate gar nicht mehr gibt; der Ausgleich stimmt
dann nicht. Option 1 (empfohlen): Nakama arbeitet unter 44,1 kHz gar nicht erst, lässt das
Signal unangetastet durch und sagt warum — du hörst exakt das Original statt einer falschen
Korrektur. Option 2: Nakama arbeitet auch dort, mit einem an die Rate angepassten Rechenbereich.
**Empfehlung: Option 1**, weil die Regeln 44,1 kHz ohnehin als kleinste Rate nennen und jede
Erweiterung eine zweite, getrennt zu beweisende Rechenart mitbringt.

---

## 8. F12 — Warmer Recall eingeschwungener dynamischer Bänder

*Feldzeile: `roh/phase-16-abdeckung.md:20` · Rohbericht: `roh/phase-16-d-dsp.md:28`, `:96-97`
und `:106` · Zusammenfassung: `BEFUNDE.md:759` (Abschlussstatus)*

### 8.1 Quellkette, zitiert

**Station 1 — der Same-Instance-Ladestart.**
`eq-copilot/plugin/sonde/SondeProcessor.cpp:1040-1062`:

```
        const bool eigenerStand = ! zustand.nurLesen && zustand.hatParameters
            && transaktion->ladestart (zustand.dspDto(), (std::uint64_t) zustand.stateRevision,
                                       zustand.undoRing, zustand.undoCursor, grund);
        ...
        if (dspKern->samplerate() > 0.0)
            publikationOffen = ! dspAusfuehrung->publiziereWirksam (transaktion->wirksam(), true);
```

Der Kern wird dabei **nicht** neu vorbereitet; er läuft weiter. `:1066-1068` nennt die
M-08/M-84-Reihenfolge ausdrücklich.

**Station 2 — der Neustart über `prepareToPlay`** (`:232-264`) ruft dagegen
`dspKern->bereiteVor (sichereRate, maxBlock)` (`:256`) — dort beginnt jeder Hüllkurven- und
Filterzustand kalt.

**Gemessene Größenordnung (Phase 16, echter Kern):** Neu vorbereiteter Kern gegen warmen
identischen Kern, Plateau −12 dB, Attack 0,1 / 10 / 500 ms: erster 10-ms-Ausgang
**+6,240277 / +6,247436 / +7,267682 dB**, Sekunde 1 bis 2 wieder gleich. Kniefall,
Threshold −15 dB: **+2,426507 / +4,814102 / +5,968904 dB**, bei Attack 500 ms in Sekunde 1 bis 2
noch **+0,267720 dB**. Echter `ladestart` mit erzwungener Publikation bei unveränderter
Topologie: Audio und double-Tap **bitgleich** zum ununterbrochenen Vergleichskern
(`roh/phase-16-d-dsp.md:28`, `:96`).

### 8.2 Die Zusage

M-84, `docs/beweise/SONDE-015.md:1162`:

> „Der Reload rekonstruiert **denselben `state_hash`** und innerhalb numerischer Toleranz
> denselben Audioausgang. Reihenfolge: State lesen → validieren → Committed setzen →
> Hostparameter synchronisieren → erster Block."

Die Zusage existiert also — aber sie nennt **weder den Referenzbeginn** (zwei Starts mit gleicher
Vorgeschichte oder Fortsetzung eines bereits warmen Signals) **noch eine Zahl** für „numerische
Toleranz". Genau diese zwei Lücken sind F12.

### 8.3 Heutige Abdeckung

- **Bein B7** `EqCopTransactionTest`, Fall `reload_rekonstruiert_denselben_audioausgang (M-84)`,
  `eq-copilot/plugin/tests/TransactionTestMain.cpp:2170-2191`. Beide Instanzen starten
  **von gleichem kaltem Beginn** (`:2183-2186`: `b->setStateInformation(...)`,
  `setRateAndBufferSizeDetails`, `prepareToPlay`), und der Zustand trägt **nur statische Bänder**
  (`:2173-2174`, `setzeBand (z, 0, 800.0, 7.0)` und `setzeBand (z, 3, 6000.0, -5.0)`). Der Test
  fordert **Bitgleichheit**, nicht eine Toleranz — er ist damit strenger als M-84 und misst
  trotzdem den offenen Fall nicht.
- **Behauptungszeile** `tools/beweise.ps1:728` (B7) sagt nur „ein Reload rekonstruiert Hash und
  Ausgang" — ohne Referenzbeginn, ohne Toleranz.
- **Register:** F12 hat keine eigene Zeile; NAK-311 führt „warmer Recall" als Sammelpunkt.

### 8.4 Urteil

**PRÄZISIERT.** Die Ausgangsvermutung („nach dem Laden ist alles kalt") ist **widerlegt**: der
Same-Instance-Ladestart bleibt bitgleich. Bestätigt ist nur die **Definitionslücke** in M-84,
und sie wird sichtbar, sobald ein neuer Kern oder ein `prepareToPlay` dazwischenliegt.

**Kategorievorschlag: LÜCKE, Schwere low.** Kein gemessener Defekt, kein persistenter
Parameterverlust; eine Zusage ohne Bezugspunkt und ohne Zahl. Klassen K3 und K4.

### 8.5 Kleinster Fix und Ort des Rotbeweises

**Fix.** M-84 in zwei getrennte, bezifferte Sätze fassen, statt eine unbestimmte Toleranz zu
tragen:

1. **Persistente Wahrheit, bitgenau:** gleicher `state_hash`, und bei zwei Instanzen mit
   **gleichem kaltem Beginn** bitgleicher Ausgang. Das ist die heutige Zusage und der heutige
   Test — beides bleibt.
2. **Audiohistorie, ausdrücklich nicht persistent:** ein neu vorbereiteter Kern beginnt mit
   leerer Hüllkurve und leeren Filterzuständen. Er schwingt mit den eingestellten
   Attack-/Hold-/Release-Zeiten ein; die zulässige Abweichung gegenüber einem durchlaufenden Kern
   wird beziffert, zum Beispiel „nach `max(attack_ms + release_ms, 200 ms)` innerhalb 1 dB, nach
   1 s innerhalb 0,1 dB". Der heute gemessene Rest von +0,267720 dB in Sekunde 1 bis 2 bei
   Attack 500 ms muss von der gewählten Zahl abgedeckt sein oder sie reißen — das ist die
   eigentliche Festlegung.

**Kein Code im Audiopfad.** Die Hüllkurvenleistung zu persistieren wäre ein neues Zustandsfeld
mit Vertrag, Migration und Fixtures — und es hieße, ein geladenes Projekt „erinnere" einen Pegel,
den es gerade nicht gibt.

**Rotbeweis.** Bein **B7**, neuer Fall neben `TransactionTestMain.cpp:2170-2191`: derselbe
gespeicherte Zustand, aber mit einem **dynamischen** Band (Attack 500 ms, Threshold im Knie),
gemessen über die erste Sekunde nach dem Reload gegen einen durchlaufenden Vergleichskern;
Erfolgskriterium ist die unter 2. bezifferte Toleranz. Zweiter Fall: Same-Instance-`ladestart`
mit erzwungener Publikation bleibt **bitgleich** (heute belegt, hält die Widerlegung fest).
Behauptungszeile `tools/beweise.ps1:728`, die um Referenzbeginn und Toleranz zu ergänzen ist.

### 8.6 Trennung Technik / Produkt

**Kein Produktentscheid nötig.** Drei Gründe, alle am HEAD belegt:

1. M-84 (`SONDE-015.md:1162`) **gewährt** bereits ausdrücklich eine „numerische Toleranz" — die
   Richtung ist entschieden, nur die Zahl fehlt. Eine Zahl ist Technik.
2. Die Invariante „State bleibt verlustfrei" (CLAUDE.md) betrifft **persistente** Werte. Die
   Hüllkurvenleistung ist Audiohistorie, kein persistenter Wert; sie wird beim Ladestart
   bewusst nicht mitgeführt (`SondeProcessor.cpp:1040-1062` kennt kein Historienfeld,
   `reset()` bei `:287-309` behandelt Audiohistorie ausdrücklich getrennt vom bestätigten
   Zustand).
3. Der gemessene Effekt liegt im ersten Moment nach dem Laden — ein Zeitpunkt, an dem in FL noch
   kein Transport läuft. Der Restwert nach einer Sekunde beträgt 0,267720 dB im ungünstigsten
   gemessenen Fall.

**Ohne Antwort baubar:** alles unter 8.5 — geschärfte M-84-Fassung, bezifferte Toleranz, der
B7-Fall, die ergänzte Behauptungszeile, dazu eine datierte Zeile in `docs/offene-punkte.md`, die
Referenzbeginn und Toleranz festhält.

**Keine neue Karte nötig**, und keine der vorhandenen U44 bis U47 betrifft F12.

---

## 9. Was ich nicht prüfen konnte

- **Keine Messung.** Alle Zahlenwerte oben (dB-Beträge, Spitzen, Abweichungen) stammen aus den
  Auditberichten der Phasen 15 und 16 und sind hier **nicht** nachgemessen; der Auftrag verbot
  Bau, Testlauf und Kanon. Geprüft habe ich, dass die zitierten Quellstellen am HEAD existieren,
  das Zitierte tatsächlich sagen und die Mechanik tragen, die die Zahlen erklärt. Die
  Arithmetik von T3-15-11 (8 / 48000, 8 / 44100, 15 / 48000, 15 / 44100) habe ich selbst
  nachgerechnet.
- **Kein Hostlauf.** Ob FL ein Monobus-Layout für Probeeq überhaupt aushandelt und ob eine
  aktuelle FL-Konfiguration Raten unter 44,1 kHz anbietet, ist nicht belegt — Phase 16 sagt das
  für beides ausdrücklich (`roh/phase-16-d-dsp.md:32`, `:55`). Die Erreichbarkeitsangaben oben
  stützen sich auf die Quelle (zugelassenes Layout, Host-Parameter), nicht auf einen FL-Lauf.
- **Die Verträglichkeit des W07-Fixes mit W03 ist hergeleitet, nicht gemessen.** Dass der
  Crossfade-Weg die neue Bank kalt startet, steht in E-8 (`SONDE-015.md:3400`) und im Code
  (`DspKern.cpp:397-414`); wie stark der Einbruch im konkreten Shelf-Fall ausfällt, ist nicht
  gemessen.
- **Die Kosten des Mono-2×2-Zweigs sind geschätzt**, nicht profiliert. Er läuft im Worker
  (`RtWache::meldeAbleitung`, `DspProgramm.cpp:89`, zählt jeden Lauf im Audiopfad), 121
  Gitterstellen × höchstens acht Bänder — eine Messung fehlt.
- **Die Höhe eines Auto-Gain-Deckels** (T3-15-09 Teil b) habe ich bewusst offen gelassen: sie ist
  Technik, braucht aber eine Messung über typische Kurven, und die war nicht erlaubt.
- **`docs/plan/fragen.json` nicht geändert.** Die Vorschläge zu neuen Karten und zu
  Textkorrekturen an U44, U46 und U47 stehen nur hier.
