# NAK-312 — Quellvalidierung Teil A (lesender Opus-Agent, HEAD 090d0fa1)

**Ticket:** NAK-312 · Planschritt S25k „Tiefenaudit 3 abarbeiten, Teil 4: Hostkante und
Plugin-Lebenslauf" · Etappe 0 „Quellvalidierung", **Teil A**
**HEAD:** `090d0fa10ce44fa8c6c9ac795982584c6ebcefb7` (Zweig `master`)
**Datum:** 20.09.2026 · **Modell:** Opus (lesend)
**IDs (8):** T3-05-01 und T3-05-02 (W02); T3-01-02, T3-01-03, T3-01-05 (W05);
T3-01-04, T3-01-09, T3-01-10 (W06). Dazu die Produktkarte **U48**.
**Quelle der Befunde:** `docs/audits/2026-09-15-tiefenaudit/BEFUNDE.md` (Phasenzeilen
`:344-347`, `:351-352`, `:411-412`; Skeptikerzeilen Phase 10 `:496-499`, `:503-504`,
`:527-528`; Phase-16-Abschnitt `:704-792`; Arbeitsübersicht W02 `:805`, W05 `:808`,
W06 `:809`) und die Rohberichte unter `docs/audits/2026-09-15-tiefenaudit/roh/`:
`phase-01-echtzeit.md`, `phase-05-hostkante.md`, `phase-10-skeptiker.md`,
`phase-11-synthese.md`, `phase-16-c-persistenz.md`, `phase-16-e-last-lebenslauf.md`,
`phase-16-f-gegenpruefung-c.md`, `phase-16-f-gegenpruefung.md`, `phase-16-abdeckung.md`.
**Gate:** Schritt S25k in `docs/plan/plan.json:358-364`; Registerzeile
`docs/offene-punkte.md:316`.
**Muster:** `docs/beweise/roh/NAK-311-quellvalidierung-teil-A.md` (Form, Tiefe, Kopf) und
`docs/beweise/roh/NAK-311-quellvalidierung-teil-B.md` (Trennung Technik und Produkt,
Kartenprüfung).

**Gate-Text wörtlich** (`docs/plan/plan.json:361`, Feld `text`, der hier einschlägige
Anfang und die Grenze):

> „ANGELEGT 18.09.2026 (Register NAK-312; Arbeitsübersicht Phase 16 W02, W05, W06, W09,
> W22, W23, W24). INHALT: 14 Befunde an der Hostkante und im Plugin-Lebenslauf — T3-05-01
> und T3-05-02 (alte Hostmailbox überschreibt geladenen Klang, read-only verarbeitet
> Hostparameter), T3-01-02, T3-01-03, T3-01-05 (JUCE/APVTS-Locks vor der RT-Wache,
> State-Lock beim Offlineflag, Automation am Wandzeittakt), T3-01-04, T3-01-09, T3-01-10
> (erstes Offline-Audio, geerbter Hostbypass, Gen-Reset bei aktiver Hörmarkierung) …
> Produktentscheide als Karten: Bypass- und Reset-Historie, MainProject-Erhalt im
> Legacy-Zustand, Sicht- oder Annahmekapazität. GRENZE: Audio-Thread ohne Sperren und
> Allokationen; State bleibt verlustfrei, fremde Bytes bytegleich; Designteil bleibt
> geparkt (keine neue Oberfläche, nur nötiger Handgriff); keine Legacy-Umbenennung.
> BEWEIS: docs/beweise/NAK-312.md mit Matrix, Rotbeweisen an B2, B6, B7, B14, B15,
> RT-Wache vor Parameterverarbeitung, vollem Kanon GRÜN und Laufzeit-Arm."

**Prüfform:** ausschließlich Quellenlektüre am HEAD. Kein Bau, kein Testlauf, kein Kanon,
kein Skript, kein FL Studio, kein Codex, keine Fehlerinjektion. Git nur lesend. Jede
Zeilennummer unten ist am HEAD `090d0fa1` selbst nachgelesen, keine aus dem Audit
übernommen. Sicherheitsbefunde (T3-12-\*) sind nicht Gegenstand; Legacy-Bezeichner
(`EQ-Copilot`, `EqCop*`, `Eqcp`) sind kein Befund; der Designteil ist geparkt
(12.09.2026) — unten steht nur Funktions- und Klangverhalten, keine Oberfläche.

**Arbeitsbaum, geprüft mit `git status --short`.** *Zu Beginn des Laufs* und *unmittelbar
vor dem Schreiben dieser Datei* identisch: die zwei fremden untracked Ordner
`briefing-hub/` und `nimbalyst-local/` sowie die zwei Auftragsdateien
`docs/beweise/roh/NAK-312-quellvalidierung-teil-A-auftrag.txt` und
`…-teil-B-auftrag.txt`, sonst nichts; HEAD beide Male `090d0fa1`. Der HEAD ist während
des Laufs **nicht** gewandert; ein Basisabgleich der Zeilennummern war deshalb nicht
nötig. Diese Datei ist die einzige Schreibaktion des Agenten;
`docs/beweise/roh/NAK-312-quellvalidierung-teil-B.md` gehört dem parallelen Teil B und
wurde nicht angefasst.

**Basis der Auditmessungen gegen HEAD.** Die Phase-16-Pakete maßen am Stand
`aff2d8188f33a9525aec1869449773bfdbe305a6`; seither liegt NAK-309 und NAK-311 dazwischen.
`git diff --stat aff2d818 090d0fa1 -- eq-copilot/plugin/{sonde,src,state,dsp,tests}
tools/beweise.ps1` nennt 27 Dateien. Für die acht IDs entscheidend:

- **`eq-copilot/plugin/sonde/SondeProcessor.cpp` ist nur an zwei Stellen verändert**
  (NAK-311 R-311-3 in `prepareToPlay`, R-311-16 im Ratenzweig, plus der Kommentar in
  `reset`). `parameterValueChanged`, `dspKontrollTakt`, `setStateInformation`,
  `setNonRealtime`, `hostParameterAbgleichen`, `fuehreTransaktionAus`, `workerLauf` und
  `getStateInformation` sind **wörtlich unverändert**. Alle Aussagen der Pakete C und F zu
  T3-05-01, T3-05-02, T3-01-02, T3-01-03 und T3-01-05 gelten damit am HEAD; nur die
  Zeilennummern sind um bis zu 34 gewandert und unten frisch gelesen.
- **`eq-copilot/plugin/src/PluginProcessor.{h,cpp}` ist nur an der v2-Pipenamenwahl
  verändert** (NAK-309 M-24/R-309-7). Kein `reset`, kein Bypass, keine Hörmarkierung.
  T3-01-09 und T3-01-10 gelten unverändert.
- **`eq-copilot/plugin/src/HoerMarkierung.h` steht nicht in der Diffliste**: die
  Zeilennummern des Audits gelten dort bitgenau weiter.
- **`eq-copilot/plugin/dsp/DspKern.cpp` ist stark verändert** (+310 Zeilen, NAK-311 W01,
  W03, W07, W35). Jede DSP-Zeile unten ist deshalb neu gelesen; die Zeilennummern des
  Audits (`DspKern.cpp:375`, `:391`, `:980`, `:1062`) treffen am HEAD **nicht** mehr.

---

## Ergebnis in einer Zeile je ID

| ID | Urteil | Kategorievorschlag | Schwere | Rotbeweis fällt an | Produktentscheid nötig |
|---|---|---|---|---|---|
| T3-05-01 (W02) | BESTÄTIGT, präzisiert: die Hashhälfte von M-84 hält, nur die Audiohälfte bricht; `prepareToPlay` heilt nicht | DEFEKT | high | Bein B7 `TransactionTestMain.cpp:2180-2200` (M-84-Block), neuer Fall mit Ladestart in **dieselbe** Instanz; Behauptungszeile `tools/beweise.ps1:728` | **nein** — M-84 und CLAUDE.md:201 entscheiden |
| T3-05-02 (W02) | BESTÄTIGT, präzisiert: Originalbytes bleiben erhalten, nur die Neutralitätszusage bricht | DEFEKT | high | Bein B7 `TransactionTestMain.cpp:2017-2036` (M-92-Block), Hostparameter **nach** dem Laden; Behauptungszeile `tools/beweise.ps1:728` | **nein** — M-92 entscheidet; der Rückweg der Hostanzeige ist Technik |
| T3-01-02 (W05) | BESTÄTIGT, präzisiert: zwei Sperrfamilien, konkreter Gegenspieler ist `hostParameterAbgleichen`; die Wache ist bauartbedingt blind | DEFEKT, dazu eine zu weite Behauptungszeile | medium (Audit: high) | Bein B6 `DspGoldenTestMain.cpp:6611-6626` (M-47-Sperrenzähler), neue Messung **oberhalb** `DspKern::verarbeite`; Behauptungszeile `tools/beweise.ps1:723` | **nein** — reine Technik |
| T3-01-03 (W05) | BESTÄTIGT, präzisiert: nur offline, aber in **jedem** Offlineblock; keine Sperrenordnungsumkehr | DEFEKT | medium (Audit: high) | Bein B7 `TransactionTestMain.cpp:2216-2234` (M-120-Block), Sperrenzähler um `setNonRealtime (true)`; `tools/beweise.ps1:728` | **nein** — reine Technik |
| T3-01-05 (W05) | BESTÄTIGT, Schwere heraufgesetzt: im Offline-Render fällt der größte Teil der Hostwerte aus | DEFEKT gegen M-120 **und** gegen den Rückfallsatz des Entwurfs | high (Audit: high) | Bein B7, neuer Fall am **echten Prozessor** neben `TransactionTestMain.cpp:2042-2060`; `tools/beweise.ps1:728` | **nein** — M-120 und Entwurf §44.3 entscheiden |
| T3-01-04 (W06) | **PRÄZISIERT**: Quellkette trägt am HEAD vollständig; die Erreichbarkeit hängt an einer ungemessenen FL-Reihenfolge | DEFEKT gegen M-120, Erreichbarkeit offen | medium (Audit: high) | Bein B7 `TransactionTestMain.cpp:2216-2234`, die acht verworfenen Blöcke in `:2228` streichen; `tools/beweise.ps1:728` | **nein** — M-120 entscheidet |
| T3-01-09 (W06) | BESTÄTIGT, präzisiert: Probeeq **misst** im Hostbypass auch nichts mehr; der interne Bypass erhält den Zustand ebenfalls nicht | LÜCKE, Produktentscheid | medium | Bein A16 `SondeNullTestMain.cpp` (Hostbypass-Eintritt) plus B6 `DspGoldenTestMain.cpp:2985-3029` als Gegenfall; `tools/beweise.ps1:688` und `:723` | **ja** — U48, Text ist zu berichtigen |
| T3-01-10 (W06) | BESTÄTIGT, präzisiert: der eigentliche Verlust ist das **offene Interventionsintervall**, nicht die Audiohistorie | LÜCKE, Produktentscheid | medium | Bein A3 `MarkierungTestMain.cpp` (neuer Fall: aktive Markierung → `reset()`); Behauptungszeile `tools/beweise.ps1:509` | **ja** — es gibt heute **keine** Karte dafür |

---

## 1. T3-05-01 — ein vor dem Recall ausstehender Hostwert überschreibt den geladenen Klang (W02)

*Befundzeile: `BEFUNDE.md:411` · Skeptikerzeile Phase 10: `:527` · Phase-16-Präzisierung:
`:735` · Arbeitsübersicht W02: `:805` · Rohbelege: `roh/phase-05-hostkante.md:26-37`,
`roh/phase-16-c-persistenz.md:13-21`, `roh/phase-16-f-gegenpruefung-c.md:11`*

### a) Behauptung des Audits

Ein vor dem State-Reload eingetroffener, noch nicht abgeholter Hostwert überschreibt nach
dem Laden den wirksamen DSP-Zustand, weil der Ladestart nur das AutomationOverlay leert,
nicht die Prozessor-Mailbox (`BEFUNDE.md:411`).

### b) Quellkette am HEAD

**Station 1 — die Mailbox nimmt an, ohne Schloss und ohne Ordnung.**
`eq-copilot/plugin/sonde/SondeProcessor.cpp:1330-1338`:

```
void SondeProcessor::parameterValueChanged (int index, float neuNormiert)
{
    // Laeuft auch im Audiothread: nur Atomics, kein Schloss, keine Allokation.
    if (abgleichTiefe > 0 || index < 0 || index >= nakama::parameter::kHostParameter)
        return;
    hostWert[(size_t) index].store (neuNormiert, std::memory_order_relaxed);
    hostEreignis[(size_t) index].fetch_add (1, std::memory_order_relaxed);
    hostEreignisOffen.store (true, std::memory_order_relaxed);
}
```

Die drei Felder liegen in `SondeProcessor.h:541-544`; der Quittierungszähler
`hostEreignisGesehen` steht in `:543` und wird an **genau zwei** Stellen berührt
(`SondeProcessor.cpp:1460` lesen, `:1461` schreiben) — es gibt keinen dritten Ort, der ihn
oder `hostWert` zurücksetzt.

**Station 2 — der Ladestart leert das Overlay, nicht die Mailbox.**
`eq-copilot/plugin/state/NakamaTransaktion.cpp:265-268` in `Transaktionskern::ladestart`:

```
    // Beide Overlays sind fluechtig (§44.4) und ueberleben keinen Ladestart.
    automationOverlay.gesetzt.fill (false);
    if (automationOverlay.laeuft) { automationOverlay.laeuft = false; ++automationOverlay.epoche; }
    beendePreview();
```

Aufgerufen wird das aus `SondeProcessor.cpp:1081-1089`. `setStateInformation`
(`:1051-1106`) berührt `hostWert`, `hostEreignis`, `hostEreignisOffen` und
`hostEreignisGesehen` an **keiner** Stelle; geleert werden dort nur `gesteOffen` und
`gesteBeteiligt` (`:1093-1094`).

**Station 3 — der Hostabgleich ersetzt den alten Mailboxwert ausdrücklich nicht.**
`SondeProcessor.cpp:1422-1432`:

```
void SondeProcessor::hostParameterAbgleichen (const nakama::parameter::Satz& werte)
{
    const AbgleichHerkunft herkunft;
    for (int i = 0; i < nakama::parameter::kHostParameter; ++i)
    {
        auto* p = hostParameter[(size_t) i];
        const float soll = p->convertTo0to1 (hostWertAus (i, werte[(size_t) i]));
        if (p->getValue() != soll)
            p->setValueNotifyingHost (soll);
    }
}
```

`AbgleichHerkunft` ist der thread-lokale Zähler aus `SondeProcessor.cpp:76-82`; er ist
genau der Grund, warum die eigene Rückmeldung in `:1333` früh zurückkehrt und `hostWert`
auf dem **alten** Wert stehen bleibt. Gerufen wird `hostParameterAbgleichen` am Ende des
Ladens in `:1103`, **außerhalb** des Zustandsschlosses.

**Station 4 — der nächste Workerzug wendet den alten Wert an.**
`SondeProcessor.cpp:1455-1465`:

```
    if (hostEreignisOffen.exchange (false, std::memory_order_relaxed))
    {
        for (int i = 0; i < nakama::parameter::kHostParameter; ++i)
        {
            const auto n = hostEreignis[(size_t) i].load (std::memory_order_relaxed);
            if (n == hostEreignisGesehen[(size_t) i]) continue;
            hostEreignisGesehen[(size_t) i] = n;
            transaktion->automationSchreiben (i, zelleAusHost (i, hostWert[(size_t) i].load (std::memory_order_relaxed)));
```

`zelleAusHost` (`:1394-1420`) gibt den bestätigten Wert nur zurück, wenn der Hostwert ihm
**in Hostgenauigkeit gleicht** (`:1403`) oder nicht endlich ist (`:1408`); −9 dB gegen +3 dB
ist beides nicht. `automationSchreiben`
(`eq-copilot/plugin/state/NakamaTransaktion.cpp:316-324`) setzt das Overlaybit, und
`wirksam()` (`:340-348`) legt den Wert über den bestätigten Zustand. Publiziert wird in
`SondeProcessor.cpp:1476-1477`.

**Station 5 — die Wirkung im Audio.** `eq-copilot/plugin/dsp/DspProgramm.cpp:260` liest
`aus.outputTrimDb = w[2].zahl;`, `:271-272` bildet daraus den linearen Faktor. Der
bestätigte Zustand, die Revision und der Hash bleiben dabei unverändert — `ladestart`
setzt `r = r0 = revision` (`NakamaTransaktion.cpp:255-256`), und
`automationSchreiben` erzeugt keine Revision.

**Eigene Ergänzung, in der Befundzeile nicht enthalten: `prepareToPlay` heilt nicht.**
`SondeProcessor.cpp:232-297` setzt Rate, Kanalzahl und Kern neu auf und publiziert in
`:284` wieder `transaktion->wirksam()` — also **einschließlich** des inzwischen aus der
alten Mailbox geschriebenen Overlays. Die Mailbox wird auch dort nicht geleert. Ein
Host, der nach dem Laden neu vorbereitet, beseitigt den Befund folglich nicht.

**Gegenrichtung, ebenfalls nachgelesen.** `reset()` (`SondeProcessor.cpp:315-343`) bewahrt
die Mailbox **absichtlich**; der Kommentar `:335-337` sagt das wörtlich: „ebenso die
Hostwert-Mailbox (`hostWert`, `hostEreignis`): sie traegt Hostwerte, keine Audiohistorie."
Für `reset` ist das richtig. Der Defekt ist, dass diese Entscheidung ungeschrieben auf den
**Ladestart** mit übergegangen ist, wo sie nicht trägt.

**Erreichbarkeit: Stufe 1.** `v1.global.output_trim_db` ist Hostparameter
(`eq-copilot/plugin/state/NakamaParameter.cpp:51`), FL automatisiert ihn; der Reload ist
der gewöhnliche Projektladevorgang. Kein Gen, keine Bedienfläche nötig.

### c) Zusage wörtlich

- SONDE-015 M-84, `docs/beweise/SONDE-015.md:1162`: „Der Reload rekonstruiert **denselben
  `state_hash`** und innerhalb numerischer Toleranz denselben Audioausgang. Reihenfolge:
  State lesen → validieren → Committed setzen → Hostparameter synchronisieren → erster
  Block."
- `CLAUDE.md:201-203`: „**State bleibt verlustfrei.** … jede persistente Änderung meldet
  dem Host Dirty-State."

**Präzisierung.** M-84 hat zwei Hälften. Die Hashhälfte **hält** am HEAD (der bestätigte
Zustand ist der geladene). Gebrochen ist nur die Audiohälfte, und zwar nicht „innerhalb
numerischer Toleranz" verfehlt, sondern um den vollen Betrag des ausstehenden Wertes.
`CLAUDE.md:201` ist **nicht** gebrochen: es geht kein State verloren. Der Befund ist damit
enger und zugleich eindeutiger als die Befundzeile ihn führt.

### d) Heutige Abdeckung

| Prüfstelle | Was sie misst | Warum sie den Befund nicht fängt |
|---|---|---|
| B7 `TransactionTestMain.cpp:2180-2200`, Fall `reload_rekonstruiert_denselben_audioausgang` (M-84); Bein `tools/beweise.ps1:728` | speichert aus Instanz `a`, lädt in eine **frisch erzeugte** Instanz `b` (`:2192-2193`), vergleicht Hash und 300 Blöcke bitgleich | Instanz `b` hat nie einen Hostwert empfangen — ihre Mailbox ist leer. Der Befund braucht dieselbe Instanz. |
| B7 `TransactionTestMain.cpp:2042-2060` (Hostautomation am echten Prozessor) | treibt Hostwerte über `hostSchreibt` (`:1795`) und `kontrollTaktFuerTest`, vergleicht gegen einen Kern ohne Automation | kein Ladestart zwischen Schreiben und Kontrolltakt |
| A16 `SondeNullTestMain.cpp`, Gate-7-Kette (`tools/beweise.ps1:688`) | Laden, read-only, Originalbytes | kein ausstehender Hostwert, kein Audio nach dem Laden |

Zwischen beiden gemessenen Nachbarn liegt genau der ungeprüfte Fall: **Hostwert → Recall in
dieselbe Instanz → Kontrolltakt.**

### e) Urteil: BESTÄTIGT

Die Quellkette schließt am HEAD ohne Lücke; sie ist gegenüber dem Auditstand
`aff2d818` in keiner der fünf Stationen verändert (Diff oben). Die Zahl
−8,99999944679 dB aus `roh/phase-16-c-persistenz.md:15` konnte ich auftragsgemäß nicht
nachfahren; sie ist als ausgeführter Komponentenlauf belegt und mit der Quellkette
konsistent (−9 dB linear ≈ 0,35481, die Restabweichung ist die float-Genauigkeit des
Hostparameters in `:1411` `p->convertFrom0to1 (normiert)`).

Zwei Präzisierungen gegenüber der Befundzeile:

1. Nur die **Audiohälfte** von M-84 bricht (c). Hash, Revision und Undo-Ring sind korrekt.
2. `prepareToPlay` beseitigt den Befund **nicht** (b, eigene Ergänzung). Die Befundzeile
   nennt als Ausgang nur „nächster dspKontrollTakt"; tatsächlich trägt ihn auch jeder
   spätere Prepare weiter, weil `:284` dasselbe Overlay publiziert.

Weder NAK-309 noch NAK-311 haben hier etwas geändert.

### f) Kategorie und Schwere

**DEFEKT, high.** Es bricht eine geschriebene Matrixzusage (M-84) an einem Stufe-1-Eingang:
zwei gewöhnliche FL-Handgriffe (Automation schreiben, Projekt laden) genügen, und der
Klang folgt danach einem Wert, den der geladene Stand nicht enthält. Hörbar bis ±24 dB
Trim beziehungsweise ein eingeschalteter EQ. Ich teile die `high`-Einstufung des Audits.

### g) Kleinster Fix

**Ort:** `SondeProcessor::setStateInformation`, `eq-copilot/plugin/sonde/SondeProcessor.cpp:1051-1106`.

**Idee.** Der Ladestart bekommt eine **Mailboxbarriere**: innerhalb desselben
Zustandsschlosses, in dem `ladestart` läuft (`:1067-1098`), wird die Prozessor-Mailbox auf
den geladenen Stand quittiert — `hostEreignisGesehen[i] = hostEreignis[i]` für alle 112
Indizes, und `hostEreignisOffen` fällt. Damit gilt: jedes Ereignis, das **vor** dem
Ladestart eingetroffen ist, ist verbraucht; jedes danach eingetroffene erhöht den Zähler
erneut und wirkt normal. Das ist eine Schleife über 112 Atomics unter einem Schloss, das
ohnehin gehalten wird: keine Allokation, kein Vertragswechsel, kein neues Feld im State.
Die Reihenfolge ist wesentlich — die Quittierung muss **vor** `hostParameterAbgleichen`
(`:1103`) stehen, sonst quittiert sie die Ereignisse, die dieser Abgleich selbst auslöst,
und der Befund verschiebt sich nur.

**Mitzuprüfende Beziehung:** **speichern↔laden.** `getStateInformation` (`:1024-1028`) muss
weiterhin den bestätigten Stand liefern und darf vom Overlay nichts sehen; ein frisches
Hostereignis **nach** dem Laden muss weiterhin wirken (Gegenfall, den der Fix nicht
brechen darf). Dazu **aktivieren↔abklingen**: `automationRuht` (`:1470-1473`) zählt in
verarbeiteten Audiosamples; die Barriere darf die laufende Epoche nicht beenden, ohne
`automationRuht` zu rufen, sonst bleibt `automationOverlay.laeuft` ohne Epochenwechsel
stehen. `ladestart` erledigt das bereits (`NakamaTransaktion.cpp:267`) — der Fix darf die
Reihenfolge nicht umdrehen.

### h) Ort des Rotbeweises

**Bein B7** (`EqCopTransactionTest`, Behauptungszeile `tools/beweise.ps1:728`), Datei
`eq-copilot/plugin/tests/TransactionTestMain.cpp`, neuer Unterfall unmittelbar hinter dem
M-84-Block `:2180-2200`.

Aufbau: 48 kHz, Block 256; `eq_enabled` an und Output-Trim +3 dB über eine echte
Transaktion committen; Audio fahren; `getStateInformation`; danach über `hostSchreibt`
(`:1795`) −9 dB schreiben, **ohne** `kontrollTaktFuerTest`; dieselben Bytes über
`setStateInformation` in **dieselbe** Instanz laden; dann `kontrollTaktFuerTest`; Audio
messen. Assertionen:

- `wirksamerZustand()` Output-Trim **exakt +3,0 dB** — heute −9,0, also rot. Die Zeile, die
  die Zusage trägt, ist `SondeProcessor.cpp:1462` (`automationSchreiben` aus `hostWert`):
  eine Mutation, die dort den alten Wert schreibt, muss den Test reißen.
- Ausgang/Eingang über 512 Samples nach dem Einschwingen innerhalb 1e-6 von
  10^(3/20) — heute 10^(−9/20);
- `bestaetigterZustand()` weiter +3,0 dB und `stateHashText()` gleich dem der Quelle
  (hält heute schon, sichert die Hashhälfte von M-84);
- **Gegenfall 1**, der den Fix begrenzt: ein Hostwert **nach** dem Laden muss wirken
  (−6 dB schreiben, Kontrolltakt, `wirksamerZustand()` = −6);
- **Gegenfall 2**: derselbe Ablauf mit `prepareToPlay` zwischen Laden und Kontrolltakt —
  heute ebenfalls rot (siehe b), nach dem Fix grün;
- **Gegenfall 3**: `reset()` zwischen Schreiben und Kontrolltakt darf den Wert **behalten**
  (`SondeProcessor.cpp:335-337` ist eine bewusste Entscheidung und muss stehen bleiben).

Der Harnisch braucht keine Schlafheuristik: `kontrollTaktFuerTest` (`SondeProcessor.h:263`)
ist die deterministische Barriere, die `roh/phase-16-c-persistenz.md:19` fordert. Die
Behauptungszeile `tools/beweise.ps1:728` muss den Same-Instance-Recall mit ausstehendem
Hostwert ausdrücklich nennen.

---

## 2. T3-05-02 — read-only bewahrt die Bytes, verliert aber die Audioneutralität (W02)

*Befundzeile: `BEFUNDE.md:412` · Skeptikerzeile Phase 10: `:528` · Phase-16-Präzisierung:
`:735` · Arbeitsübersicht W02: `:805` · Rohbelege: `roh/phase-05-hostkante.md:39-45`,
`roh/phase-16-c-persistenz.md:23-27`, `roh/phase-16-f-gegenpruefung-c.md:12`*

### a) Behauptung des Audits

Ein unbekanntes State-Major bleibt read-only mit Originalbytes, lässt sich danach aber über
Hostautomation hörbar aktivieren; die Transaktions-API prüft `nurLesen`, der
Hostmailbox-Konsum und die wirksame DSP-Publikation nicht (`BEFUNDE.md:412`).

### b) Quellkette am HEAD

**Station 1 — read-only wird gesetzt und lädt neutral.**
`eq-copilot/plugin/sonde/SondeProcessor.cpp:1081-1089`:

```
        const bool eigenerStand = ! zustand.nurLesen && zustand.hatParameters
            && transaktion->ladestart (zustand.dspDto(), (std::uint64_t) zustand.stateRevision,
                                       zustand.undoRing, zustand.undoCursor, grund);
        if (! eigenerStand)
        {
            const bool neutral = transaktion->ladestart (nakama::parameter::DspSatz {}, 0, {}, 0, grund);
```

**Station 2 — die Sperre sitzt allein am Transaktionseingang.**
`SondeProcessor.cpp:1236-1243`:

```
        if (zustand.nurLesen)
        {
            ergebnis.ausgang  = nakama::transaktion::Ausgang::fehler;
            ergebnis.revision = transaktion->revision();
            ergebnis.hash     = transaktion->hash();
            ergebnis.grund    = "schreibgeschuetzt";
            return ergebnis;
        }
```

Das ist die **einzige** Stelle im Prozessor, die `zustand.nurLesen` als Schranke benutzt.
`parameterValueChanged` (`:1330-1338`) und `dspKontrollTakt` (`:1447-1478`) lesen das Feld
nicht.

**Station 3 — die nichttopologischen Werte gehen trotz `samplegenau == false` durch.**
`eq-copilot/plugin/state/NakamaTransaktion.cpp:340-348`:

```
param::DspSatz Transaktionskern::wirksam() const
{
    auto s = committed;
    const auto& t = param::tabelle();
    for (int i = 0; i < param::kHostParameter; ++i)
        if (automationOverlay.gesetzt[(size_t) i] && (samplegenau || ! t[(size_t) i].topologisch))
            s.werte[(size_t) i] = automationOverlay.werte[(size_t) i];
    return s;
}
```

`samplegenau` ist im Produkt fest false (`SondeProcessor.cpp:155`
`transaktion->setzeSamplegenaueAutomation (false);`). Entscheidend ist damit
`! topologisch`, und für beide beteiligten Parameter ist das wahr:
`eq-copilot/plugin/state/NakamaParameter.cpp:79` führt
`boolean ("v2.global.eq_enabled", false, false, true, 2)` — das dritte Argument ist
`topologisch` (Signatur `:15-16`); `:51` führt
`gleitkomma ("v1.global.output_trim_db", -24.0, 24.0, 0.0)`, und **jeder** `gleitkomma`
setzt `b.topologisch = false` (`:30`).

**Station 4 — die Wirkung im Programm.** `eq-copilot/plugin/dsp/DspProgramm.cpp:263`
`aus.eqEngagiert = w[(size_t) param::kIndexEqEnabled].b;` und `:272`
`aus.outputTrimLin = aus.outputTrimDb == 0.0 ? 1.0 : dbInLinear (aus.outputTrimDb);`.
Damit rechnet der Pfad: `DspKern::istPassthrough` (`eq-copilot/plugin/dsp/DspKern.cpp:456-460`)
ist nicht mehr wahr, und `verarbeiteStueck` nimmt nicht den Rückweg.

**Station 5 — die Bytes bleiben tatsächlich unberührt.** `SondeProcessor.cpp:1036-1048`:

```
    auto stand = zustand;
    if (! stand.nurLesen && stand.hatParameters)
        transaktion->schreibeIn (stand);
    return stand;
```

Bei `nurLesen` schreibt niemand in den Stand; `getStateInformation` (`:1024-1028`) gibt ihn
unverändert zurück. Die Behauptung des Audits, es gehe **kein** Byte verloren, hält am HEAD.

**Erreichbarkeit: Stufe 1.** Beide Parameter sind unter den 112 Hostparametern
(`SondeProcessor.cpp:157-164`, `NakamaParameter.h`); FL kann sie ohne jede Nakama-Fläche
automatisieren.

### c) Zusage wörtlich

SONDE-015 M-92, `docs/beweise/SONDE-015.md:1175`: „Er bleibt **read-only mit Originalbytes**
wie heute: **audio-neutral**, keine Bindung, `getStateInformation` liefert die Bytes
unverändert zurück."

Dazu `CLAUDE.md:201-202`: „Unbekannte Major-Versionen oder unzulässige Matrizen werden
read-only mit Originalbytes gehalten."

**Präzisierung.** M-92 nennt drei Eigenschaften. „Originalbytes" und „keine Bindung"
**halten** am HEAD (Station 5; `v3Status` sperrt die berichtete Bindung in
`SondeProcessor.cpp:1144-1150`). Gebrochen ist ausschließlich **„audio-neutral"**.
`CLAUDE.md:201` ist nicht gebrochen.

### d) Heutige Abdeckung

Bein B7 (`tools/beweise.ps1:728`), Fall
`fremdes_major_laedt_neutral_und_bleibt_schreibgeschuetzt`,
`eq-copilot/plugin/tests/TransactionTestMain.cpp:2017-2036`. Der Fall lädt
`eq-copilot/fixtures/state/schema2/fremdes-major-3.bin` (`:2021`), prüft `nurLesen`, den
neutralen bestätigten Zustand, die Ablehnung einer Transaktion mit Grund
`schreibgeschuetzt` und `zurueck == fremd` (`:2031-2033`). Danach folgt **kein einziger
Hostparameterschreibvorgang** — `hostSchreibt` kommt im ganzen Block nicht vor, und Audio
wird nicht gefahren. Der Test ist grün und für den Befund blind.

Bein B2 (`EqCopStateMigrationTest`, `tools/beweise.ps1:693`) prüft die Statehälfte
(„unbekanntes Major read-only mit Originalbytes") und kennt den Prozessor nicht.

A16 (`tools/beweise.ps1:688`) fährt die Gate-7-Kette am gebauten Bundle mit
`measurement_position=post_fader_contribution`: derselbe read-only-Eingang, ebenfalls ohne
anschließende Hostautomation.

### e) Urteil: BESTÄTIGT

Die Kette schließt am HEAD ohne Lücke; alle fünf Stationen sind gegenüber `aff2d818`
unverändert. Der gemessene Wert +6,00000017258 dB
(`roh/phase-16-c-persistenz.md:25`) ist mit Station 4 konsistent
(10^(6/20) = 1,995262…, die Restabweichung ist die float-Genauigkeit des Hostparameters).

Präzisierung: nur die Neutralitätshälfte von M-92 bricht (c). Die Befundzeile sagt das
bereits selbst („der Befund behauptet daher keinen Byteverlust",
`roh/phase-05-hostkante.md:45`); die Lektüre bestätigt es.

### f) Kategorie und Schwere

**DEFEKT, high.** M-92 ist eine geschriebene Zusage mit genau diesem Wortlaut, und der Weg
ist Stufe 1. Die Wirkung ist schwerer als bei einem gewöhnlichen Fehlwert: der User hat
einen Stand geladen, den dieses Build **nicht versteht**, und bekommt trotzdem
Verarbeitung — das ist das Grundgesetz „nichts Ungefragtes" (`CLAUDE.md:29`) in seiner
schärfsten Form, denn hier ist nicht einmal bekannt, was der Stand bedeuten sollte. Ich
teile `high`.

### g) Kleinster Fix

**Ort:** `Transaktionskern::wirksam` (`eq-copilot/plugin/state/NakamaTransaktion.cpp:340-348`)
oder, schmaler, `SondeProcessor::dspKontrollTakt` (`SondeProcessor.cpp:1447-1478`).

**Idee.** Der read-only-Zustand ist eine Eigenschaft des **Standes**, nicht der
Transaktions-API; er gehört deshalb dorthin, wo der wirksame Satz entsteht. Der
Transaktionskern bekommt ein `nurLesen`-Flag, das `ladestart` aus dem geladenen Stand
mitbekommt; `wirksam()` gibt dann den (neutralen) bestätigten Satz zurück, ohne das
Overlay anzulegen. Das ist eine zusätzliche Bedingung in `:345` und ein `bool` im Kern:
keine Allokation, kein Echtzeitbezug, kein Vertrags- oder Stateformatwechsel.

**Was der Fix zusätzlich tun muss, sonst lügt der Host.** Wird das Overlay nur ignoriert,
steht der FL-Regler auf +6 dB, während nichts passiert — ein Bedienelement, das einen
Zustand vorspiegelt, den es nicht gibt (`CLAUDE.md:51`, sinngemäß auch für den Hostregler).
Der Fix muss deshalb im selben Zug `hostParameterAbgleichen` (`:1422-1432`) mit dem
neutralen Satz rufen, sodass der Hostregler sichtbar zurückspringt. Das ist der Punkt, an
dem die Produktwirkung sichtbar wird; technisch ist der Weg vorhanden und wird in
`gestusAbschliessen` (`:1381-1391`) für den abgewiesenen Gestus bereits genauso benutzt.

**Mitzuprüfende Beziehung:** **speichern↔laden** (Originalbytes müssen bytegleich bleiben —
der Fix darf `gehaltenerStand` nicht anfassen) und **aktivieren↔abklingen**: der Rückweg
aus read-only heraus. Lädt der User danach einen gültigen Stand, muss die Automation
wieder wirken; `ladestart` setzt das Flag also in beide Richtungen.

### h) Ort des Rotbeweises

**Bein B7** (`tools/beweise.ps1:728`), `eq-copilot/plugin/tests/TransactionTestMain.cpp`,
M-92-Block `:2017-2036`, unmittelbar hinter `:2035`.

Aufbau: dieselbe Fixture laden; Audio fahren und den Ausgang als Referenz nehmen; dann über
`hostSchreibt` (`:1795`) `v2.global.eq_enabled` auf 1 und `v1.global.output_trim_db` auf
+6 dB schreiben; `kontrollTaktFuerTest`; Audio messen. Assertionen:

- Ausgang **bitgleich** zum Eingang über 512 Samples — heute ×1,9953, also rot. Die Zeile,
  die die Zusage trägt, ist `NakamaTransaktion.cpp:345`: eine Mutation, die dort die
  `nurLesen`-Bedingung entfernt, muss den Test reißen.
- `zustandLesen().nurLesen` weiter true und `getStateInformation` bytegleich zur Fixture
  (hält heute, sichert die Bytehälfte von M-92);
- der Hostparameter steht nach dem Kontrolltakt wieder auf seinem neutralen Wert
  (die Anzeigehälfte des Fixes; heute steht er auf +6);
- **Gegenfall**: derselbe Ablauf mit einem **gültigen** eigenen Stand — die Automation muss
  weiterhin wirken. Ohne diesen Gegenfall repariert der Fix die Neutralität und bricht die
  gewöhnliche Hostautomation.

Die Behauptungszeile `tools/beweise.ps1:728` muss die Hostautomation nach dem fremden Major
ausdrücklich nennen; heute nennt sie den read-only-Weg dort gar nicht (er steht bei A16,
`:688`).

---

## 3. T3-01-02 — zwei JUCE-Sperrfamilien liegen vor der Echtzeitwache (W05)

*Befundzeile: `BEFUNDE.md:344` · Skeptikerzeile Phase 10: `:496` · Arbeitsübersicht W05:
`:808` · Rohbeleg: `roh/phase-01-echtzeit.md:45-51`*

### a) Behauptung des Audits

Zusätzlich zu den bekannten Wrapper-Sperren (NAK-48) nimmt normale Hostautomation zwei
weitere Parametersperren, bevor der atomare Nakama-Listener erreicht ist; die RT-Wache
beginnt erst in `DspKern::verarbeite` (`BEFUNDE.md:344`).

### b) Quellkette am HEAD

Die Kette liegt vollständig im **Bauartefakt** `eq-copilot/build/_deps/juce-src/` — keine
Repoquelle. Das ist für die Beweisführung wichtig und unten in g) berücksichtigt.

**Station 1 — Eintritt im Audiocallback.**
`…/juce_audio_plugin_client/juce_audio_plugin_client_VST3.cpp:3714`
`tresult PLUGIN_API process (Vst::ProcessData& data) override`, darin `:3757`
`processParameterChanges (*data.inputParameterChanges);` (Definition `:3634`), darin
`:3687` `setValueAndNotifyIfChanged (*param, (float) change->value);`.

**Station 2 — die Wertänderung ruft den Host-Benachrichtigungsweg.** Dieselbe Datei
`:843-850`:

```
static void setValueAndNotifyIfChanged (AudioProcessorParameter& param, float newValue)
{
    if (approximatelyEqual (param.getValue(), newValue))
        return;

    const InParameterChangedCallbackSetter scopedSetter { inParameterChangedCallback };
    param.setValueNotifyingHost (newValue);
}
```

**Station 3 — erste Sperre.**
`…/juce_audio_processors/processors/juce_AudioProcessorParameter.cpp:59-63`:

```
void AudioProcessorParameter::setValueNotifyingHost (float newValue)
{
    setValue (newValue);
    sendValueChangedMessageToListeners (newValue);
}
```

und `:111-117`:

```
void AudioProcessorParameter::sendValueChangedMessageToListeners (float newValue)
{
    ScopedLock lock (listenerLock);

    for (int i = listeners.size(); --i >= 0;)
        if (auto* l = listeners [i])
            l->parameterValueChanged (getParameterIndex(), newValue);
```

**Station 4 — zweite Sperre über den APVTS-Adapter.** Probeeq hält einen APVTS
(`eq-copilot/plugin/sonde/SondeProcessor.cpp:143`
`parameterBaum (*this, nullptr, "NakamaProbeeqParameter", baueParameterLayout())`). Jeder
Adapter meldet sich beim Parameter an —
`…/juce_audio_processors/utilities/juce_AudioProcessorValueTreeState.cpp:83`
`parameter.addListener (this);` — und reagiert in `:147-158`:

```
    void parameterValueChanged (int, float) override
    {
        const auto newValue = denormalise (parameter.getValue());

        if (! listenersNeedCalling && approximatelyEqual ((float) unnormalisedValue, newValue))
            return;

        unnormalisedValue = newValue;
        listeners.call ([this] (Listener& l) { l.parameterChanged (parameter.paramID, unnormalisedValue); });
```

`listeners.call` ist `:183-187`:

```
        void call (Fn&& fn)
        {
            const CriticalSection::ScopedLockType lock (mutex);
            listeners.call (std::forward<Fn> (fn));
        }
```

Die Sperre wird **vor** der (möglicherweise leeren) Hörerliste genommen; ein leerer
APVTS-Hörerbestand spart sie nicht. Bei einer echten Wertänderung — und genau die kommt
über Station 2 — ist der frühe `return` in `:152` nicht wirksam.

**Station 5 — Nakamas eigener Listener ist sauber, aber der letzte.**
`SondeProcessor.cpp:157-164` meldet alle 112 Parameter an (`:163`
`parameterListe[i]->addListener (this);`); `parameterValueChanged` (`:1330-1338`) arbeitet
nur mit Atomics, wie ihr Kommentar `:1332` sagt. Das begrenzt die Arbeit **innerhalb** des
Listeners und macht die beiden äußeren Sperren nicht rückgängig.

**Station 6 — wo die Wache beginnt.** `eq-copilot/plugin/dsp/DspKern.cpp:1057-1059`:

```
void DspKern::verarbeite (float* const* kanaele, int numKanaele, int numSamples) noexcept
{
    RtWache::Bereich wache;
```

Das ist die **einzige** Stelle im Produktcode, an der `RtWache::Bereich` steht
(repoweite Suche über `eq-copilot/plugin/`). `RtWache::meldeSperre`
(`eq-copilot/plugin/dsp/DspRtWache.h:50-53`) zählt nur, solange `imAudiopfad()` (`:38`)
gilt, und gezählt wird ohnehin nur über den Wrapper `GemeldeteSperre` (`:78-93`). Die
Wache ist damit **bauartbedingt** blind für alles, was vor `DspKern::verarbeite` liegt —
und der Kopfkommentar `DspRtWache.h:9-15` sagt das ehrlich („sie zaehlt nicht sich
selbst", „im echten Audiopfad"). Der Aufruf steht in `SondeProcessor.cpp:432`, also nach
Stempelbildung und weit nach dem Parametereingang des Wrappers.

**Eigene Ergänzung: der konkrete Gegenspieler.** Die Befundzeile nennt die Sperren, aber
keinen zweiten Halter. Es gibt ihn: `hostParameterAbgleichen`
(`SondeProcessor.cpp:1422-1432`) ruft in `:1430` `p->setValueNotifyingHost (soll)` auf
**demselben** Parameterobjekt, vom Nachrichten- beziehungsweise Workerthread aus — nach
jedem Commit (`:1260`), nach jedem abgewiesenen Gestus (`:1391`) und nach jedem Laden
(`:1103`). Dort läuft dieselbe Kette Station 3 → Station 4. Eine Prioritätsumkehr ist damit
nicht hypothetisch, sondern hat einen benannten Auslöser: ein Projektladevorgang schreibt
112 Parameter, während der Audiothread automatisierte Punkte entgegennimmt.

**Erreichbarkeit: Stufe 1.** Gewöhnliche FL-Automation auf einem der 112 Parameter.

### c) Zusage wörtlich

`CLAUDE.md:32-33`: „Audio-Thread: keine Sperren, Allokationen, Datei-, Pipe- oder
Netzzugriffe, kein Logging; Überlast verwirft Analyseframes, nie Audio."

Dazu die Matrixzeile SONDE-015 M-47, `docs/beweise/SONDE-015.md:1100`: „Im Callback gibt es
**keine Sperre, keine Allokation**, keinen Datei-, Pipe- oder Netzzugriff, kein Logging und
keine geworfene Ausnahme. Gemessen wird mit einem thread-lokalen Allokations- und einem
Sperrzähler über mindestens 4000 Blöcke wechselnder Größe …"

Und der Entwurf, `docs/FL-Nakama-Sonden-Design-Entwurf.md:3066-3067`: „State-Tree-Copy/
Replace und Parametergesten laufen nie im Audiothread."

Die Regel kennt keine Ausnahme für Sperren fremden Codes. Sie spricht vom **Thread**, nicht
vom Autor.

### d) Heutige Abdeckung

| Prüfstelle | Was sie misst | Warum sie den Befund nicht fängt |
|---|---|---|
| B6 `DspGoldenTestMain.cpp:6611-6612`, Fall `null_sperren_im_callback (M-47)`; Bein `tools/beweise.ps1:723` | `RtWache::sperren() == 0` über 4000 Blöcke | Der Zähler beginnt bei `DspKern.cpp:1059`. Er kann nicht sehen, was der Wrapper vor `DspKern::verarbeite` tut. |
| B6 `DspGoldenTestMain.cpp:6618-6626` (Selbstprobe der Wache) | eine Sperre außerhalb zählt 0, eine innerhalb zählt 1 | belegt, dass der Zähler funktioniert — und zugleich, dass er thread- und bereichsgebunden ist |
| A16 `SondeNullTestMain.cpp` (`tools/beweise.ps1:688`) | Bitgleichheit, Zähler für nicht endliche Eingänge und verworfene Analyseframes | misst Audioinhalt, keine Sperren |

Es gibt am HEAD **keinen** Prüfpunkt oberhalb von `DspKern::verarbeite`.

**Eigener Nebenbefund zur Behauptungszeile.** `tools/beweise.ps1:723` sagt: „im Callback
zaehlt der thread-lokale Zaehler ueber 4000 Bloecke wechselnder Groesse 0 Allokationen und
0 Sperren". Das Wort **„im Callback"** ist zu weit: gezählt wird der Bereich in
`DspKern::verarbeite`, nicht der Hostcallback. Die Behauptung deckt damit mehr ab, als das
Bein misst — dieselbe Art Riss, die V0 (T3-09-01) für die Frischewache benennt.

### e) Urteil: BESTÄTIGT, mit drei Präzisierungen

Alle sechs Stationen am HEAD gelesen; die Kette schließt. Präzisierungen:

1. Die Sperren liegen **ausschließlich in JUCE**, nicht in Nakama-Code. Das ändert das
   Urteil nicht (c), aber den Fix: „unseren Code lockfrei machen" ist nicht der Hebel.
2. Der **zweite Halter** ist benannt (`hostParameterAbgleichen`, b) — der Audit lässt
   offen, wer die Sperre je halten könnte, und ohne einen solchen Halter wäre der Befund
   eine reine Formalie.
3. Die Wache ist **nicht defekt**, sondern eng. Ihr eigener Kopf sagt das
   (`DspRtWache.h:9-15`). Defekt ist die Behauptungszeile `tools/beweise.ps1:723`, die mehr
   behauptet, als der Zähler sieht.

Weder NAK-309 noch NAK-311 haben hier etwas geändert; die eingebundene JUCE-Fassung ist
dieselbe wie zum Auditzeitpunkt.

### f) Kategorie und Schwere

**DEFEKT gegen `CLAUDE.md:32` und M-47, dazu eine zu weite Behauptungszeile (V0-Klasse) —
Schwere medium** (Audit: `high`).

Begründung der Herabstufung, zur Entscheidung durch den Dirigenten: die Haltedauer beider
Sperren ist kurz und beschränkt. `listenerLock` liegt **je Parameter** vor, ebenso der
Adaptermutex (`listeners` ist Mitglied von `ParameterAdapter`,
`juce_AudioProcessorValueTreeState.cpp:72-90`); es gibt keinen globalen Flaschenhals über
alle 112. Innerhalb der Sperren steht bei Probeeq nur der atomare Nakama-Listener und eine
leere APVTS-Hörerliste (Probeeq hat keinen Editor, `SondeProcessor.h:158-159`). Ein
gemessener Aussetzer liegt nicht vor, und Paket E hat bei Blockgröße 64 **0**
Dienstzeitüberschreitungen gemessen (`roh/phase-16-e-last-lebenslauf.md:32-35`), mit
laufender Hostautomation alle 20 ms (`:22`). Die `high`-Einstufung folgt der Klasse K6
(Grundgesetz), nicht einer gemessenen Wirkung. Gegen eine weitere Herabstufung spricht,
dass die Regel absolut formuliert ist und ein Halter existiert (b).

### g) Kleinster Fix

**Ort:** `SondeProcessor::hostParameterAbgleichen` (`SondeProcessor.cpp:1422-1432`) und die
Anbindung in `:157-164` — **nicht** der Wrapper.

**Idee.** Die Sperre wird gebraucht, weil derselbe Parameter von zwei Threads aus über
`setValueNotifyingHost` geht. Der Audiothread braucht diesen Weg nicht: er will nur den
Wert in die Mailbox legen. Die Nakama-Seite kann deshalb ihren Listener durch den
**Endlistener** ersetzen — JUCE führt neben `listeners` ein `finalListener`
(`juce_AudioProcessorParameter.cpp:55-57`, `:119-121`), der unter derselben Sperre läuft;
das allein hilft nicht. Wirksam ist stattdessen, die zweite Familie loszuwerden: der APVTS
wird für die 112 Hostparameter **nicht** gebraucht (Probeeq hat keinen Editor und keine
`Listener`-Registrierung; die Wahrheit ist der Transaktionskern). Werden die Parameter
statt über `AudioProcessorValueTreeState::ParameterLayout` als schlichte
`juce::RangedAudioParameter` direkt an den Prozessor gehängt, entfällt der `ParameterAdapter`
und mit ihm Station 4 vollständig. Station 3 bleibt — sie gehört JUCE und liegt vor jedem
Plugincode; sie ist ohne den in §44.3 ohnehin vorgesehenen gepinnten Wrapperpatch
(`docs/FL-Nakama-Sonden-Design-Entwurf.md:3072-3082`) nicht zu entfernen.

**Die ehrliche Zwischenstufe, die vorher kommt:** die Wache muss **messen können**, was
heute niemand misst. `RtWache::Bereich` gehört an den Anfang von
`SondeProcessor::processBlock` (`:369`) statt allein in `DspKern::verarbeite`; die
Behauptungszeile `tools/beweise.ps1:723` beziehungsweise `:688` sagt danach, was sie
wirklich deckt. Ohne diesen Schritt beglaubigt ein zu enger Zähler jeden Folgefix.

**Mitzuprüfende Beziehung:** **verbinden↔trennen** — `addListener` (`:163`) und der
Destruktor müssen paarweise bleiben; **speichern↔laden** — `hostParameterAbgleichen` ist
Teil des Ladewegs (`:1103`), ein Umbau der Parameteranbindung muss den Ladestand weiterhin
zum Host spiegeln (M-84, Abschnitt 1).

### h) Ort des Rotbeweises

**Bein B6** (`EqCopDspGoldenTest`, Behauptungszeile `tools/beweise.ps1:723`) für den
Zähler, **Bein B7** (`:728`) für den Prozessor.

Der Rotbeweis muss an der Zeile fallen, die die Zusage trägt — das ist der Eintritt des
gemessenen Bereichs, heute `DspKern.cpp:1059`. Aufbau in B7, neben
`TransactionTestMain.cpp:2042-2060`: `RtWache::zuruecksetzen()`, dann 200 Blöcke fahren und
währenddessen aus einem zweiten Thread `hostSchreibt` treiben, während der Testthread
`processBlock` ruft; die Sperren der Stationen 3 und 4 über einen gemeldeten Wrapper
(`RtWache::GemeldeteSperre`, `DspRtWache.h:78-93`) sichtbar machen. Assertionen:

- `RtWache::sperren() == 0` mit dem Bereich ab `processBlock` statt ab
  `DspKern::verarbeite` — **heute rot**, sobald ein Parameterereignis im Block liegt;
- **Gegenfall**: derselbe Lauf ohne Parameterereignisse bleibt 0 (grenzt die Messung auf
  den Parameterweg ein);
- **Gegenfall**: der Zähler sieht die Allokationen des Workers weiterhin **nicht**
  (`DspGoldenTestMain.cpp:6672-6674` misst das heute; es muss halten bleiben).

Die Mutation, die den Test reißen muss: den `RtWache::Bereich` in `processBlock` wieder
nach unten schieben.

---

## 4. T3-01-03 — das Offlineflag nimmt in jedem Block das Zustandsschloss (W05)

*Befundzeile: `BEFUNDE.md:345` · Skeptikerzeile Phase 10: `:497` · Arbeitsübersicht W05:
`:808` · Rohbeleg: `roh/phase-01-echtzeit.md:53-59`*

### a) Behauptung des Audits

Probeeq nimmt in jedem Offline-Audioblock `zustandSchloss`; derselbe Mutex schützt
State-Serialisierung und das Bauen von DSP-Programmen auf anderen Threads
(`BEFUNDE.md:345`).

### b) Quellkette am HEAD

**Station 1 — der Wrapper ruft es aus dem Callback, in jedem Block.**
`…/juce_audio_plugin_client_VST3.cpp:3881` `void processAudio (Vst::ProcessData& data)`,
darin `:3890-3892`:

```
        {
            const ScopedLock sl (pluginInstance->getCallbackLock());

            pluginInstance->setNonRealtime (data.processMode == Vst::kOffline);
```

Der Aufruf ist unbedingt und steht **innerhalb** des Callback-Schlosses. Ein zweiter,
harmloser Aufruf liegt in `setupProcessing` (`:3605`), außerhalb des Callbacks.

**Station 2 — die Override nimmt das Schloss.**
`eq-copilot/plugin/sonde/SondeProcessor.cpp:1321-1328`:

```
void SondeProcessor::setNonRealtime (bool offline) noexcept
{
    juce::AudioProcessor::setNonRealtime (offline);
    if (! offline) return;
    const juce::ScopedLock l (zustandSchloss);
    transaktion->beendePreview();
    dspKern->setzeHoermatrix (nakama::dsp::Hoermatrix::processed);
}
```

Die JUCE-Basis ist nur ein Feldschreiben
(`…/juce_AudioProcessor.cpp:410-413`). Der frühe `return` in `:1324` bedeutet: **in
Echtzeit fällt keine Sperre an**; bei Offline fällt sie in **jedem** Block, auch wenn sich
nichts geändert hat. Die zweite Zeile im Schloss (`:1327`) braucht es gar nicht —
`setzeHoermatrix` ist ein Atomic (`eq-copilot/plugin/dsp/DspKern.h:244`).

**Station 3 — was die Gegenspieler unter demselben Schloss tun.** `zustandSchloss` steht in
`SondeProcessor.h:442`. Unter ihm laufen:

- `getStateInformation` (`SondeProcessor.cpp:1024-1028`): vollständige Serialisierung über
  `nakama::state::speichere (gehaltenerStand(), ziel)`, einschließlich der Kopie des
  gehaltenen Standes (`:1036-1048`);
- `dspKontrollTakt` (`:1447-1478`): `transaktion->wirksam()` gibt einen `param::DspSatz`
  **per Wert** zurück, und der trägt `std::vector<Schutzzone> zonen`
  (`eq-copilot/plugin/state/NakamaParameter.h:151-158`) — also eine Heapallokation, sobald
  Zonen gesetzt sind — und `publiziereWirksam` baut daraus ein Programm;
- `fuehreTransaktionAus` (`:1229-1253`), `prepareToPlay` (`:260-292`), `reset` (`:340-342`),
  `releaseResources` (`:308-312`) und sechzehn Getter.

Der Audiothread kann im Offlinebetrieb also auf eine Serialisierung oder einen
Programmbau warten.

**Station 4 — Sperrenordnung, eigens geprüft: es gibt keine Umkehr.** `prepareToPlay`
(`:260-261`), `releaseResources` (`:308-309`) und `reset` (`:340-341`) nehmen **erst**
`getCallbackLock()`, **dann** `zustandSchloss`. Der Audiothread hält über den Wrapper
(`:3890`) ebenfalls erst das Callback-Schloss und will dann `zustandSchloss`. Die einzige
Stelle, die beide in umgekehrter Reihenfolge berührt, ist `v3Status`
(`SondeProcessor.cpp:1134-1195`) — dort endet der `zustandSchloss`-Block jedoch **in
`:1175`**, bevor `:1184` `getCallbackLock()` nimmt. Eine Verklemmung habe ich nicht
gefunden; der Befund ist eine Wartezeit, kein Deadlock.

**Zwei irreführende Kommentare am selben Befund.**

- `eq-copilot/plugin/sonde/SondeProcessor.h:442`:
  `juce::CriticalSection zustandSchloss;   ///< nur Nachrichten-/Hostthread, nie processBlock`
  — Station 1 zeigt, dass der Audiothread es nimmt. Buchstäblich ist es nicht
  `processBlock`, sondern der Aufruf unmittelbar davor im selben Callback; als Leseanleitung
  ist der Satz falsch.
- `eq-copilot/plugin/sonde/SondeProcessor.cpp:1181-1183`: „Der Control-Thread liest ihn
  deshalb unter JUCEs Callback-Lock; **der Audiothread nimmt nie unseren
  Runtime-/State-Lock.**" — dieselbe falsche Aussage, an der Stelle, an der sie eine
  Entscheidung begründet.

**Erreichbarkeit: Stufe 1**, aber ausschließlich im Offline-Render (Export, Freeze).

### c) Zusage wörtlich

`CLAUDE.md:32-33` (Wortlaut oben in 3c) und SONDE-015 M-47
(`docs/beweise/SONDE-015.md:1100`). Beide nennen **keine** Offline-Ausnahme. Der Entwurf
`docs/FL-Nakama-Sonden-Design-Entwurf.md:3066-3067` sagt ebenfalls unbedingt:
„State-Tree-Copy/Replace und Parametergesten laufen nie im Audiothread."

Eine Zusage, die Offline **ausnimmt**, habe ich nicht gefunden — weder in SONDE-015 §44.4
(`:3088-3095`) noch in §44.5 (`:3097-3100`).

### d) Heutige Abdeckung

| Prüfstelle | Was sie misst | Warum sie den Befund nicht fängt |
|---|---|---|
| B6 `DspGoldenTestMain.cpp:6611-6612` (M-47) | Sperrenzähler im Kernbereich | wie 3d: der Bereich beginnt erst in `DspKern.cpp:1059`; `setNonRealtime` liegt außerhalb |
| B7 `TransactionTestMain.cpp:2216-2234` (M-120) | `setNonRealtime (true)` am echten Prozessor (`:2227`), danach Audio | misst Audioinhalt, nie eine Sperre; und der Aufruf steht dort **einmal**, nicht je Block |
| A16 `SondeNullTestMain.cpp` (`tools/beweise.ps1:688`) | u. a. „im 32-kHz-Fenster … die Echtzeitwache des Kerns 0 gemeldete Sperren" (`tools/beweise.ps1:728`, B7) | derselbe enge Bereich |

Kein Bein ruft `setNonRealtime (true)` je Block und zählt dabei.

### e) Urteil: BESTÄTIGT, mit drei Präzisierungen

Die Kette ist am HEAD vollständig und gegenüber `aff2d818` unverändert. Präzisierungen:

1. **Nur offline.** Der frühe `return` in `:1324` hält den Echtzeitpfad frei. Die
   Befundzeile sagt „in jedem Offline-Audioblock" und ist damit korrekt; die
   Arbeitsübersicht `BEFUNDE.md:808` verkürzt das zu „wiederholtes Offlineflag nimmt
   State-Lock", was den Echtzeitfall offen lässt.
2. **Keine Sperrenordnungsumkehr** (Station 4). Das ist ein Ergebnis, kein Nichtbefund: ein
   Fix darf die Ordnung nicht kippen.
3. **Zwei irreführende Kommentare** (b) — der zweite begründet eine Entscheidung mit einer
   falschen Tatsache.

### f) Kategorie und Schwere

**DEFEKT gegen `CLAUDE.md:32` und M-47 — Schwere medium** (Audit: `high`).

Begründung: im Offline-Render gibt es keine Gerätefrist; ein wartender Block verlängert den
Export, er reißt kein Audioloch. Das Paket E hat den Offlinefall getrennt gemessen und dort
**0** Überschreitungen gefunden (`roh/phase-16-e-last-lebenslauf.md:35`, dort ausdrücklich
„nur rechnerischer Vergleich"). Gegen eine weitere Herabstufung spricht, dass die
Haltedauer der Gegenspieler **unbegrenzt** ist (Serialisierung, Programmbau mit
Allokation) und die Invariante keine Ausnahme kennt.

### g) Kleinster Fix

**Ort:** `SondeProcessor::setNonRealtime`, `eq-copilot/plugin/sonde/SondeProcessor.cpp:1321-1328`.

**Idee.** Die Funktion soll nur bei einem **Wechsel** arbeiten, und der Wechsel soll
sperrenfrei angemeldet werden. Ein `std::atomic<bool> offlineGesehen` merkt den zuletzt
gesehenen Wert; ist er gleich, kehrt die Funktion sofort zurück (das ist der Normalfall in
99,9 % der Offlineblöcke). Bei einem Wechsel bleibt genau zweierlei zu tun: die Hörmatrix
auf Processed stellen — das ist **schon** sperrenfrei (`DspKern.h:244`) — und die Vorschau
beenden. Letzteres braucht den Transaktionskern, gehört also nicht in den Callback: ein
zweites Atomic `previewEndeAngefordert` wird gesetzt, und `dspKontrollTakt` (`:1447-1478`)
verbraucht es unter dem Schloss, das es ohnehin hält.

**Die Kopplung, die dabei mitzudenken ist:** T3-01-04 verlangt, dass die Vorschau **vor dem
ersten Offline-Sample** weg ist. Ein Worker, der erst in bis zu 5 ms zieht, erfüllt das
nicht. Die Lösung liegt in derselben Änderung: die Hörmatrixumstellung wirkt sofort und
ohne Fade (siehe 6g), und die **Bank** der Vorschau muss der Kern selbst am Blockrand
loslassen — `beendeCandidate` (`eq-copilot/plugin/dsp/DspKern.cpp:434-443`) arbeitet bereits
nur mit Atomics und einer Publikation und kann direkt aus dem Callback gerufen werden. Der
Transaktionskern zieht sein Buchhaltungs-`beendePreview` dann beim nächsten Kontrolltakt
nach. Damit lösen T3-01-03 und T3-01-04 **einen** Änderungssatz.

Zusätzlich: die zwei irreführenden Kommentare (`SondeProcessor.h:442`,
`SondeProcessor.cpp:1183`) gehören in denselben Satz berichtigt.

**Mitzuprüfende Beziehung:** **öffnen↔schließen** — Vorschau setzen (`:1267-1271`) und
beenden (`:1273-1277`) müssen weiterhin ein Paar bilden, auch wenn das Ende jetzt aus zwei
Hälften besteht (Kern sofort, Kern-Buchhaltung beim nächsten Takt); **aktivieren↔abklingen**
— der Rückweg `setNonRealtime (false)` muss die Vorschau **nicht** wiederbeleben (das tut er
heute nicht und darf es nicht anfangen).

### h) Ort des Rotbeweises

**Bein B7** (`tools/beweise.ps1:728`), `eq-copilot/plugin/tests/TransactionTestMain.cpp`,
neuer Fall neben dem M-120-Block `:2216-2234`.

Aufbau: Prozessor mit gesetzter Vorschau und Hörmatrix Candidate; `RtWache::zuruecksetzen()`;
dann 200 Blöcke fahren, vor jedem `setNonRealtime (true)` rufen wie der Wrapper es tut;
`zustandSchloss` für diesen Lauf über `RtWache::GemeldeteSperre` melden. Assertionen:

- `RtWache::sperren() == 0` über die 200 Blöcke — **heute 200**, also rot. Die Zeile, die
  die Zusage trägt, ist `SondeProcessor.cpp:1325` (`const juce::ScopedLock l (zustandSchloss);`);
  eine Mutation, die sie wieder unbedingt macht, muss den Test reißen.
- der **erste** Aufruf darf die Vorschau beenden: `previewAktiv() == false` und
  `gewuenschteHoermatrix() == processed` nach dem ersten Block (hält heute, muss halten);
- **Gegenfall**: `setNonRealtime (false)` je Block zählt 0 Sperren (hält heute schon,
  belegt dass Echtzeit nie betroffen war);
- **Gegenfall**: `getStateInformation` parallel zu einem Offlinelauf blockiert den
  Audiothread nach dem Fix nicht mehr — messbar als Callback-Dauer ohne Ausreißer, oder
  einfacher als Sperrenzähler 0 trotz gehaltenem Schloss auf einem zweiten Thread.

---

## 5. T3-01-05 — der wirksame Automationszeitpunkt hängt am 5-ms-Wanduhrtakt (W05)

*Befundzeile: `BEFUNDE.md:347` · Skeptikerzeile Phase 10: `:499` · Arbeitsübersicht W05:
`:808` · Rohbeleg: `roh/phase-01-echtzeit.md:67-73`*

### a) Behauptung des Audits

Der Block, in dem Hostautomation hörbar wird, hängt am 5-ms-Wanduhrtakt des Workers; kurze
Parameterfolgen können vor dessen nächstem Zug zusammenfallen. Identische Hostevents und
Blöcke bestimmen daher keinen identischen Realtime-/Offline-Verlauf (`BEFUNDE.md:347`).

### b) Quellkette am HEAD

**Station 1 — der Eingang speichert nur den letzten Wert.**
`SondeProcessor.cpp:1335-1337` (Wortlaut oben in 1b): ein `store` und ein `fetch_add`. Es
gibt **keine** Folge, keinen Ring, keinen Blockbezug. Zwei Werte desselben Parameters
zwischen zwei Workerzügen überschreiben einander; nur der Zähler steigt zweimal.

**Station 2 — der Takt hängt an der Wanduhr.** `SondeProcessor.cpp:569-579`:

```
    double naechsterKontrollTakt = juce::Time::getMillisecondCounterHiRes();
    while (workerLaeuft.load())
    {
        // SONDE-015 4a: der Takt des Control-Workers - ACKs ernten,
        // Hostautomation uebernehmen, den wirksamen Zustand publizieren. Er
        // nimmt das Zustandsschloss und NIE zugleich das Analyseschloss.
        if (juce::Time::getMillisecondCounterHiRes() >= naechsterKontrollTakt)
        {
            dspKontrollTakt();
            naechsterKontrollTakt = juce::Time::getMillisecondCounterHiRes() + 5.0;
        }
```

Bei fehlender Arbeit wartet der Worker bis zu 20 ms (`:739`
`workerWarte.wait_for (l, std::chrono::milliseconds (20), …)`).

**Station 3 — erst der Zug macht den Wert wirksam.** `SondeProcessor.cpp:1455-1465` und
`:1476-1477` (Wortlaut oben in 1b). `hostEreignisGesehen` springt dabei auf den **aktuellen**
Zählerstand: übersprungene Zwischenwerte sind dauerhaft verloren, nicht aufgeschoben.

**Station 4 — der Kern rampt blockgenau, nicht wertgenau.**
`eq-copilot/plugin/dsp/DspKern.cpp:618-626` setzt die fünf Rampenziele beim Blockrand;
`kRampeSamples = 256` (`eq-copilot/plugin/dsp/DspProgramm.h:91`). Die Rampe glättet den
**Weg** zu dem Ziel, das der Blockrand gerade sieht; ein nie gesetztes Ziel kann sie nicht
nachholen.

**Station 5 — die gemessene Hostwirklichkeit.**
`eq-copilot/identity/host-capabilities-fl-v1.json:27` meldet
`"sample_accurate_automation": "unsupported"`, `:54` nennt das Rohfeld:
`automation.punkte_gesamt = 83303, max_punkte_pro_block = 1, … bloecke.blockgroesse_min = 1,
blockgroesse_max = 4096`. FL liefert also **höchstens einen Punkt je Block** — und
Blockgrößen ab 1 Sample. Bei 48 kHz sind 5 ms 240 Samples: **jede** von FL gemeldete
Blockgröße unter 240 Samples kann zwei oder mehr Werte in ein Workerfenster legen. Das ist
kein Sonderfall, sondern der Normalbetrieb bei kleinem Gerätepuffer.

**Station 6 — der Offlinefall, hergeleitet.** Ein Offline-Render läuft schneller als
Echtzeit; Paket E hat für Probeeq 3,00394 s Audio in 0,21441 s Wandzeit gemessen
(`roh/phase-16-e-last-lebenslauf.md:41`), also rund 14-fach. Bei Blockgröße 333 sind das
433 Blöcke in 0,21441 s; der Worker zieht in dieser Zeit rund 43-mal. Im selben Material
in Echtzeit läge zwischen zwei Blöcken 6,94 ms, also **mehr** als die 5-ms-Frist — fast
jeder Block bekäme seinen Zug. **Derselbe Event- und Blockverlauf führt damit in Echtzeit
zu rund 433 übernommenen Werten und offline zu rund 43.** Das ist keine Toleranz, das ist
eine andere Automationskurve. (Rechnung aus der Quelle und der gemessenen Offlinegeschwindigkeit
hergeleitet, **nicht** gefahren.)

**Irreführender Kommentar.** `SondeProcessor.cpp:151-154`: „M-119: FL meldet
`sample_accurate_automation` = unsupported …, und ein samplegenauer Pfad ist nicht gebaut.
Die **Sonde rampt blockweise**, und Topologieautomation wirkt nicht (§44.3 letzter Absatz)."
Die Sonde rampt nicht blockweise, sondern **workerzugweise**. Genau diese Verwechslung ist
der Befund.

### c) Zusage wörtlich

- SONDE-015 M-120, `docs/beweise/SONDE-015.md:1224`: „Beide erzeugen **denselben
  Parameterverlauf** und damit innerhalb numerischer Toleranz denselben Ausgang, bei
  gleichem Event- und Blockverlauf."
- **Die schärfere, spezifischere Zusage steht im Entwurf**,
  `docs/FL-Nakama-Sonden-Design-Entwurf.md:3084-3086`: „Scheitert dieser Spike, meldet die
  Probe `sample_accurate_automation=false`: kontinuierliche Werte werden ehrlich nur **vom
  vorigen zum letzten Blockwert** gerampt, Topologieautomation wird deaktiviert, und
  Realtime/Offline-Gleichheit wird nur für denselben Event-/Blockverlauf behauptet."
- Dazu `:3070-3071`: „Eine zweite freie Glättung darf die Hostautomation nicht zeitlich
  verschieben. Realtime- und Offline-Render müssen denselben Parameterverlauf erzeugen."

Der Entwurf beschreibt den **gewählten Rückfall** — und der Code erfüllt ihn nicht: gerampt
wird nicht zum letzten **Blockwert**, sondern zum letzten Wert **beim nächsten Workerzug**.
Die Befundzeile zitiert nur M-120; `:3084-3086` ist die genauere Fundstelle und war bisher
nicht genannt.

### d) Heutige Abdeckung

| Prüfstelle | Was sie misst | Warum sie den Befund nicht fängt |
|---|---|---|
| B6 `DspGoldenTestMain.cpp:8182-8348`, Fall `realtime_und_offline_gleich (M-120)`; Bein `tools/beweise.ps1:723` | zwei Kernläufe über 180 Blöcke wechselnder Größe, einer im Wanduhrtakt (`:8275-8276`), einer ohne; Ausgang bitgleich und Parameterverlauf bitgleich (`:8329-8347`) | Der Test **setzt die Bedingung selbst**, die im Produkt fehlt: `:8249-8261` ruft `uebernehmeZustand` an festen Blocknummern in **beiden** Läufen. Der Kommentar `:8189-8191` sagt es wörtlich: „Zwei frische Kerne fahren dieselbe Folge … **mit denselben Worker-Aufrufen an denselben Blockraendern**." |
| B7 `TransactionTestMain.cpp:2042-2060` | Hostautomation am echten Prozessor, dann `kontrollTaktFuerTest()` (`:2054`) | ruft den Takt von Hand, genau einmal, nach allen vier Schreibvorgängen — der Wettlauf ist per Konstruktion entschieden |
| B7 `TransactionTestMain.cpp:2216-2234` (M-120 Prozessorhälfte) | Offline nutzt den bestätigten Zustand | keine Automation während des Offlinelaufs |

Es gibt am HEAD **keinen** Fall, in dem der echte Worker den Zeitpunkt bestimmt.

### e) Urteil: BESTÄTIGT, Schwere heraufgesetzt

Die Quellkette ist am HEAD vollständig und gegenüber `aff2d818` unverändert. Zwei
Präzisierungen und eine Verschärfung:

1. **Die Zusage ist genauer als zitiert** (c): der Entwurf hat den Rückfall ausdrücklich
   auf „letzter **Blockwert**" festgelegt. Das ist die Zeile, gegen die das Verhalten
   steht — nicht nur die weichere M-120-Formel „innerhalb numerischer Toleranz".
2. **Die Erreichbarkeit ist gemessen, nicht angenommen**: FLs eigene Capability-Messung
   (`host-capabilities-fl-v1.json:54`) nennt Blockgrößen ab 1 Sample und genau einen Punkt
   je Block. Unter 240 Samples bei 48 kHz ist der Verlust der Regelfall.
3. **Der Offlinefall ist der schwere** (Station 6): bei rund 14-facher Geschwindigkeit
   überlebt etwa jeder zehnte Automationswert. Ein Export klingt damit bei bewegter
   Automation hörbar anders als das, was der User beim Abhören gehört hat. Das ist der
   eigentliche Produktschaden, und er steht in der Befundzeile nur als Möglichkeit
   („können zusammenfallen").

Was der Befund **nicht** ist: ein dauerhaft falscher Wert. Die Mailbox hält immer den
letzten Wert; nach dem Ende einer Automationsbewegung steht der richtige Endwert. Der
Schaden ist der Weg, nicht das Ziel.

### f) Kategorie und Schwere

**DEFEKT gegen M-120 und gegen `FL-Nakama-Sonden-Design-Entwurf.md:3084-3086` — Schwere
high** (Audit: `high`, mit anderer Begründung).

Begründung der Heraufhaltung gegenüber T3-01-02 und T3-01-03: dies ist der einzige der drei
W05-Befunde mit einer **hörbaren** Folge auf dem ausgelieferten Produkt, und zwar genau an
der Stelle, an der ein Musiker sie am wenigsten verzeiht — im Export.

### g) Kleinster Fix

**Ort:** `SondeProcessor::processBlock` (`SondeProcessor.cpp:369-495`) und
`SondeProcessor::dspKontrollTakt` (`:1447-1478`).

**Idee.** Der Übernahmezeitpunkt muss am **Block** hängen, nicht an der Wanduhr. Der
Audiothread darf dafür weder ein Programm bauen noch eine Sperre nehmen — er kann aber
**anzeigen**, dass ein Blockrand vorbeigekommen ist: ein `std::atomic<std::uint64_t>
blockZaehler`, den `processBlock` vor `dspKern->verarbeite` (`:432`) erhöht, und ein
zweites Atomic, das der Worker als „letzter gesehener Block" führt. Der Worker zieht dann
nicht mehr *alle 5 ms*, sondern *sobald ein neuer Block vorliegt und Hostereignisse offen
sind*; die 5-ms-Frist bleibt als **Obergrenze** für die übrigen Aufgaben (ACK-Ernte,
Ruhegrenze) bestehen. `workerWarte` (`:739`) wird dazu aus `processBlock` heraus
benachrichtigt — was eine Signalisierung im Audiothread wäre und deshalb **nicht** geht;
stattdessen verkürzt der Worker seine Wartezeit, solange `hostEreignisOffen` steht.

Das beseitigt den Verlust nicht vollständig — zwischen zwei Workerzügen können weiterhin
Blöcke liegen — aber es macht den Zeitpunkt **blockgebunden statt wanduhrgebunden**, und
damit ist der Entwurfssatz `:3084-3086` erfüllt und Realtime/Offline identisch. Die
vollständige Erfüllung („jeder Blockwert wird genommen") verlangt einen vorallokierten
Ereignisring je Parameter, den der Worker leert — das ist derselbe Ring, den §44.3
(`:3072-3082`) für den gepinnten Wrapperpatch ohnehin vorsieht, und gehört in dasselbe
Ticket wie die Brücke, nicht in diesen Fix.

Der irreführende Kommentar `SondeProcessor.cpp:153` gehört in denselben Satz berichtigt.

**Mitzuprüfende Beziehung:** **aktivieren↔abklingen** — die Ruhegrenze
(`:1468-1473`, `kAutomationsRuheSekunden = 0,25`,
`eq-copilot/plugin/state/NakamaTransaktion.h:62`) zählt in verarbeiteten Audiosamples und
muss bei geänderter Taktung dieselbe Epochenfolge liefern (M-81, zwei Epochenwechsel je
Geste — B7 misst das heute, `tools/beweise.ps1:728`). Dazu **speichern↔laden**: das Overlay
darf weiterhin keine Revision erzeugen.

### h) Ort des Rotbeweises

**Bein B7** (`tools/beweise.ps1:728`), `eq-copilot/plugin/tests/TransactionTestMain.cpp`,
neuer Fall neben `:2042-2060` — bewusst am **Prozessor**, nicht am Kern: der Kern ist
nachweislich sample- und blockdeterministisch (B6 `:8329-8347`), der Fehler sitzt eine
Schicht darüber.

Aufbau: zwei Prozessoren mit identischem Stand, 48 kHz, Blockgröße 64 (1,33 ms — unter der
5-ms-Frist). In beiden dieselbe Eventfolge auf `v1.global.output_trim_db`: Block 10 → +6 dB,
Block 11 → 0 dB, Block 12 → +6 dB, Block 13 → 0 dB. Prozessor A bekommt den echten Worker
mit seiner Wanduhr (oder eine kontrollierte Barriere, die den Takt erst nach Block 13
zieht); Prozessor B bekommt `kontrollTaktFuerTest()` nach **jedem** Block. Assertionen:

- Ausgang von A und B über den ganzen Lauf **bitgleich** — heute rot, weil A nur den letzten
  Wert sieht. Die Zeile, die die Zusage trägt, ist `SondeProcessor.cpp:1460`
  (`if (n == hostEreignisGesehen[(size_t) i]) continue;`): eine Mutation, die dort den
  Zählervergleich entfernt, muss den Test reißen — denn dann wird jeder Wert genommen.
- `automationEpoche()` in beiden Läufen gleich (schützt die Epochenzusage M-81);
- **Gegenfall**: bei Blockgröße 4096 (85 ms, weit über der Frist) sind A und B schon heute
  bitgleich — grenzt die Regel auf das ein, was sie treffen soll;
- **Gegenfall**: ein Lauf ohne Automationsereignisse ist bitgleich (schützt gegen einen Fix,
  der den Takt insgesamt verändert);
- **Gegenfall Offline**: derselbe Lauf mit `setNonRealtime (true)` muss denselben Ausgang
  liefern wie in Echtzeit — das ist M-120 wörtlich und heute die schwerste Hälfte.

Die Behauptungszeile `tools/beweise.ps1:728` nennt heute die Prozessorhälfte von M-120 nicht
mit dem Wort „Zeitpunkt"; sie muss sagen, dass der wirksame Block blockgebunden ist.

---

## 6. T3-01-04 — die ersten Offline-Samples tragen noch die Vorschau (W06)

*Befundzeile: `BEFUNDE.md:346` · Skeptikerzeile Phase 10: `:498` · Arbeitsübersicht W06:
`:809` · Rohbeleg: `roh/phase-01-echtzeit.md:61-65`*

### a) Behauptung des Audits

Beim direkten Wechsel aus einer laufend gehörten Vorschau in Offline bleibt Candidate
während der Rückblende im Ausgang; der zugehörige Test verwirft genau den Beginn des
Offline-Renders (`BEFUNDE.md:346`).

### b) Quellkette am HEAD

**Station 1 — der Wechsel setzt nur den Wunsch.** `SondeProcessor.cpp:1321-1328`
(Wortlaut in 4b): `dspKern->setzeHoermatrix (processed)` ist
`eq-copilot/plugin/dsp/DspKern.h:244`
`void setzeHoermatrix (Hoermatrix h) noexcept { hoerwunsch.store (h, std::memory_order_release); }`
— ein Wunsch, kein Zustand.

**Station 2 — der Wunsch wird zu einem Fade.**
`eq-copilot/plugin/dsp/DspKern.cpp:1233-1239`:

```
    if (wirksam != hoerLaufend)
    {
        if (hoerFadeRest <= 0)
        {
            hoerVorher   = hoerLaufend;
            hoerLaufend  = wirksam;
            hoerFadeRest = kFadeSamples;
        }
```

`kFadeSamples = 256` (`eq-copilot/plugin/dsp/DspProgramm.h:87`) — 5,33 ms bei 48 kHz.

**Station 3 — beim ersten Sample ist das Gewicht der Vorschau genau 1.**
`eq-copilot/plugin/dsp/DspKern.cpp:1339-1352`:

```
    for (size_t i = 0; i < schreibBis; ++i)
    {
        double l = 0.0, r = 0.0;
        hoerWert (hoerLaufend, i, l, r);

        if (hoerFadeRest > 0)
        {
            // Linear, wie jeder andere Uebergang des Kerns (§5.2 Feinheit 2).
            double lv = 0.0, rv = 0.0;
            hoerWert (hoerVorher, i, lv, rv);
            const double t = 1.0 - (double) hoerFadeRest / (double) kFadeSamples;
            l = lv * (1.0 - t) + l * t;
```

Bei `i == 0` ist `hoerFadeRest == kFadeSamples`, also `t = 0` und
`l = lv * 1,0 + l * 0,0` — **ausschließlich** `hoerVorher`, und das ist Candidate. Das
erste ausgegebene Offline-Sample ist zu 100 % die Vorschau.

**Station 4 — die Candidate-Bank bleibt dafür absichtlich stehen.**
`DspKern.cpp:549-556`:

```
        // X-1 (Entscheid E-33): mischt die Hoermatrix noch Candidate-Anteile,
        // ist IHRE Rueckblende die einzige Blende. Die Bank fadet dann nicht
        // parallel nach Dry, sondern klingt unveraendert weiter und dient
        // erst aus, wenn die Hoermatrix sie loslaesst (`beendeHoerHalt`) -
        // sonst nullten zwei identische A/B-Zustaende nicht (M-55).
        const bool hoerHalt = p == Pfad::candidate && hoermatrixMischtCandidate();
```

`hoermatrixMischtCandidate` (`:494-498`) ist während des Fades wahr, weil
`hoerVorher == candidate` und `hoerFadeRest > 0`.

**Station 5 — die Gegenkräfte, die den Fall auflösen können.** Beide sind Hostwege, nicht
Plugincode:

- `bereiteVor` (`DspKern.cpp:97-162`) setzt in `:146-150` `hoerLaufend`, `hoerVorher` und
  `hoerFadeRest` hart auf Processed / 0;
- `freigeben` (`:164-195`) tut in `:186-190` dasselbe.

Der VST3-Wrapper ruft `prepareToPlay` **nur** aus `setActive (true)`
(`…/juce_audio_plugin_client_VST3.cpp:2833`, über `preparePlugin (…, CallPrepareToPlay::yes)`,
`:3948-3958`) und `releaseResources` aus `setActive (false)` (`:2836-2838`).
`setupProcessing` ruft ausdrücklich **kein** `prepareToPlay` (`:3607`,
`CallPrepareToPlay::no`).

**Damit hängt die Erreichbarkeit an der FL-Reihenfolge:** deaktiviert FL das Plugin vor
einem Render (`setActive(false)` → `setupProcessing(kOffline)` → `setActive(true)`), räumt
`releaseResources`/`prepareToPlay` den Fade weg und der Befund fällt nicht. Schaltet FL den
`processMode` im laufenden Betrieb um — wofür JUCE den Aufruf in `:3892` überhaupt je Block
führt —, fällt er. **Welche der beiden Reihenfolgen FL fährt, ist nirgends gemessen**;
`eq-copilot/identity/host-capabilities-fl-v1.json` enthält dazu kein Feld, und
`docs/beweise/termin-a/` und `termin-b/` messen Parameter-, Kontext- und Latenzfragen, keine
Aktivierungsreihenfolge.

### c) Zusage wörtlich

SONDE-015 M-120, `docs/beweise/SONDE-015.md:1224`: „**Offline-Render läuft mit dem
bestätigten Zustand, nie mit einer Vorschau.**"

Dazu der Entwurf `docs/FL-Nakama-Sonden-Design-Entwurf.md:2137`: „Preview-Zustand wird nie
im Projekt gespeichert und **nie in einen Offline-Render übernommen**." Und `:2748`:
„Preview, Focus und Delta sind im Offline-Render neutral."

Drei Fundstellen, alle unbedingt („nie"). Eine Fadetoleranz nennt keine davon.

**Irreführender Kommentar.** `eq-copilot/plugin/sonde/SondeProcessor.h:176-179`:
„Offline-Render laeuft mit dem bestaetigten Zustand, nie mit einer Vorschau (§44.4, §49.2
Gate 3, M-120): der Wechsel in den Offline-Betrieb beendet die Preview und **stellt die
Hoermatrix auf Processed**." Er liest sich wie eine sofortige Umstellung; tatsächlich ist es
ein 256-Sample-Fade (Station 2).

### d) Heutige Abdeckung

Bein B7 (`tools/beweise.ps1:728`), Fall `offline_render_nutzt_den_bestaetigten_zustand`,
`eq-copilot/plugin/tests/TransactionTestMain.cpp:2216-2234`. Der Fall stellt genau diesen
Übergang her (`:2224-2227`) und verwirft ihn dann:

```
        a->setNonRealtime (true); b->setNonRealtime (true);
        fahreAudio (*a, 8, 256, 51); fahreAudio (*b, 8, 256, 51);
        const auto ya = fahreAudio (*a, 100, 256, 52);
```

`:2228` fährt acht Blöcke zu 256 Samples — 2048 Samples, achtmal die Fadelänge — **bevor**
`:2229-2230` den Vergleichsbereich aufzeichnet. Der Test ist grün und für den Befund blind.
Das ist dieselbe Abdeckungslücke, die `BEFUNDE.md:498` benennt; am HEAD steht sie bei
`:2228` statt bei `:2219`.

B6 `DspGoldenTestMain.cpp:8182-8348` (M-120, Kernhälfte) fährt den Hörmatrixwechsel als
Teil seiner Folge (`:8255`, `:8257`), vergleicht aber zwei Läufe **gegeneinander**, nicht
gegen den bestätigten Zustand — beide tragen denselben Fade.

### e) Urteil: PRÄZISIERT

**Was gesichert ist (aus der Quelle am HEAD):**

1. Die Umstellung der Hörmatrix ist ein 256-Sample-Fade, und das erste Sample trägt zu
   100 % die Vorschau (Stationen 1 bis 3). Die Zahl folgt zwingend aus `:1349`
   (`t = 1 − hoerFadeRest/kFadeSamples`) und `:1239`.
2. Die Vorschaubank bleibt dafür absichtlich stehen (Station 4).
3. Der B7-Fall kann es nicht sehen (d).
4. Drei Zusagen sagen „nie" (c).

**Was ich nicht klären konnte:** ob FL vor einem Offline-Render deaktiviert. Genau daran
hängt, ob der Befund heute im Produkt auftritt oder ein latenter Defekt ist, der beim
nächsten Hostwechsel scharf wird. Die Befundzeile führt den Fall als Stufe 1; die
Arbeitsübersicht `BEFUNDE.md:809` stellt ihn bereits hinter V1 („V1 für Hostreihenfolge"),
und die Skeptikerzeile `roh/phase-01-echtzeit.md:65` sagt es selbst: „Ein Host, der vor dem
Render erneut vollständig vorbereitet, kann die Historie vorher beseitigen; dieser Bericht
behauptet deshalb keinen bei jeder FL-Exportfolge auftretenden Fehler." Ich schärfe das:
die **zwei** möglichen Reihenfolgen sind oben benannt (Station 5), und die Messung ist ein
Einzeiler im Laufzeit-Arm (protokollieren, ob `prepareToPlay` zwischen dem letzten
Echtzeitblock und dem ersten Offlineblock läuft).

Die Arbeit und der Rotbeweis sind in beiden Fällen dieselben; **die Messung gehört vor den
Fix, nicht statt seiner** — dasselbe Muster wie bei T3-01-01 in NAK-311.

### f) Kategorie und Schwere

**DEFEKT gegen M-120 — Schwere medium** (Audit: `high`).

Begründung der Herabstufung, zur Entscheidung durch den Dirigenten: die Wirkung ist auf
256 Samples am Anfang eines Renders begrenzt und setzt voraus, dass im Augenblick des
Wechsels eine Vorschau **hörbar** war. Ein Render, der an einer Stille beginnt, zeigt nichts.
Gegen eine weitere Herabstufung spricht, dass die Zusage dreimal unbedingt formuliert ist und
dass ein Export das Artefakt dauerhaft einbrennt. Die Vorschau ist außerdem der Ort, an dem
der User bewusst etwas hört, das **nicht** bestätigt ist — sie in ein Ergebnis zu tragen,
trifft das Grundgesetz (`CLAUDE.md:29`).

### g) Kleinster Fix

**Ort:** `eq-copilot/plugin/dsp/DspKern.cpp:1233-1255` (Hörmatrixwechsel) und
`SondeProcessor::setNonRealtime` (`SondeProcessor.cpp:1321-1328`).

**Idee.** Der Kern bekommt neben `hoerwunsch` ein `std::atomic<bool> hoerHartSchalten`.
Steht es, nimmt der Wechsel in `:1235-1239` `hoerFadeRest = 0` statt `kFadeSamples`: die
Umstellung gilt ab dem ersten Sample. `setNonRealtime (true)` setzt beide Atomics. Ein
Schnitt ist hier richtig und kein Klick im Wortsinn — der Übergang liegt am **Anfang** des
Renders, nicht mittendrin, und M-120 verlangt genau das. Ein laufender Fade in die andere
Richtung muss dabei mit beendet werden (`hoerVorher = hoerLaufend`), sonst bleibt ein
Mischstand stehen.

Zusammen mit 4g ist das **ein** Änderungssatz: `setNonRealtime` wird sperrenfrei, wirkt nur
beim Wechsel, stellt die Hörmatrix hart um und lässt die Candidate-Bank über
`beendeCandidate` (`DspKern.cpp:434-443`, nur Atomics und eine Publikation) los; die
Buchhaltung des Transaktionskerns zieht der nächste Kontrolltakt nach.

Der Kommentar `SondeProcessor.h:176-179` gehört in denselben Satz berichtigt.

**Mitzuprüfende Beziehung:** **öffnen↔schließen** — der Hörhalt (`beendeHoerHalt`,
`DspKern.cpp:500-517`) muss die gehaltene Candidate-Bank auch dann freigeben, wenn der Fade
nie lief; heute hängt er an `hoermatrixMischtCandidate()` (`:503`), das bei `hoerFadeRest == 0`
sofort falsch wird — der Fix muss also `beendeHoerHalt` im selben Block erreichen, sonst
bleibt eine Bank hängen (genau der Fall, den E-33 verhindern soll). Dazu
**aktivieren↔abklingen**: ein Wechsel **zurück** nach Candidate (Offline aus, wieder
Vorschau hören) muss weiterhin weich sein — das Atomic gilt nur für die Offlinerichtung.

### h) Ort des Rotbeweises

**Bein B7** (`tools/beweise.ps1:728`), `eq-copilot/plugin/tests/TransactionTestMain.cpp`,
Fall `offline_render_nutzt_den_bestaetigten_zustand` bei `:2216-2234`.

**Der Fall existiert bereits und ist nur an einer Zeile blind.** `:2228` verwirft acht
Blöcke. Der Rotbeweis ist, diese Zeile zu streichen und ab dem **ersten** Sample nach
`setNonRealtime (true)` zu vergleichen. Assertionen:

- `ya` und `yb` bitgleich **ab Sample 0** — heute weichen die ersten 256 Samples ab, also
  rot. Die Zeile, die die Zusage trägt, ist `DspKern.cpp:1239`
  (`hoerFadeRest = kFadeSamples;`): eine Mutation, die dort 0 schreibt, macht den Test grün
  und belegt, dass er genau diese Zeile misst.
- Zusätzlich die Betragsschranke: `max |ya[i] − yb[i]|` über die ersten 256 Samples ist 0.
- **Gegenfall 1**: derselbe Ablauf ohne aktive Vorschau ist schon heute bitgleich ab
  Sample 0 — grenzt die Regel ein.
- **Gegenfall 2**: `setNonRealtime (false)` nach dem Render und Rückkehr zu Candidate blendet
  weiterhin **weich** (Nachbarsprung innerhalb der E-31-Schranke,
  `docs/beweise/SONDE-015.md:3425`) — schützt M-55 gegen eine pauschale Reparatur „nie
  faden".
- **Gegenfall 3**: nach dem Wechsel darf keine Bank hängen bleiben — `pflege()` gibt die
  Candidate-Bank innerhalb weniger Blöcke frei (misst den `beendeHoerHalt`-Riss aus g).

**Vor dem Fix** gehört eine Messung in den Laufzeit-Arm
(`tools/fl/laufzeit.ps1`, Szenario unter `docs/gesundheit/szenarien/`): protokollieren, ob
FL zwischen dem letzten Echtzeit- und dem ersten Offlineblock `prepareToPlay` oder
`releaseResources` ruft. Das entscheidet die Erreichbarkeitsstufe, nicht die Reparatur.

---

## 7. T3-01-09 — der Hostbypass ist ein anderer Einstieg als `v1.global.bypass` (W06)

*Befundzeile: `BEFUNDE.md:351` · Skeptikerzeile Phase 10: `:503` · Kartenentwurf PF11-01:
`:211-215` · Arbeitsübersicht W06: `:809` · Rohbelege: `roh/phase-01-echtzeit.md:95-99`,
`roh/phase-16-e-last-lebenslauf.md:53`*

### a) Behauptung des Audits

Der VST3-Hostbypass geht an Nakamas Hard-Bypass-Fade vorbei: beide Prozessoren erben den
unmittelbaren JUCE-Passthrough. Für Hostbypass fehlen eine Klangübergangs- und eine
Historienregel (`BEFUNDE.md:351`).

### b) Quellkette am HEAD

**Station 1 — keiner der beiden Prozessoren übernimmt den Bypass.** Repoweite Suche über
`eq-copilot/plugin/` nach `getBypassParameter` und `processBlockBypassed`: **kein Treffer**.
`SondeProcessor` (`SondeProcessor.h:131-134`) und `EqCopilotProcessor`
(`eq-copilot/plugin/src/PluginProcessor.h:76`) überschreiben beides nicht.

**Station 2 — der Wrapper legt sich deshalb einen eigenen Bypassparameter an.**
`…/juce_audio_plugin_client_VST3.cpp:689-697`:

```
        bool vst3WrapperProvidedBypassParam = false;
        auto* bypassParameter = audioProcessor->getBypassParameter();

        if (bypassParameter == nullptr)
        {
            vst3WrapperProvidedBypassParam = true;
            ownedBypassParameter.reset (new AudioParameterBool ("byps", "Bypass", false));
            bypassParameter = ownedBypassParameter.get();
        }
```

und hängt ihn in `:705-706` an die exportierte Parameterliste. FL sieht also einen
**113. VST3-Parameter**, der mit Nakamas eigenem `v1.global.bypass`
(`eq-copilot/plugin/state/NakamaParameter.cpp:49`) nichts zu tun hat.

**Station 3 — die Weiche im Callback.**
`…/juce_audio_plugin_client_VST3.cpp:3904-3909`:

```
                // processBlockBypassed should only ever be called if the AudioProcessor doesn't
                // return a valid parameter from getBypassParameter
                if (pluginInstance->getBypassParameter() == nullptr && comPluginInstance->getBypassParameter()->getValue() >= 0.5f)
                    pluginInstance->processBlockBypassed (buffer, midiBuffer);
                else
                    pluginInstance->processBlock (buffer, midiBuffer);
```

Weil Station 1 `nullptr` liefert, ist der erste Zweig aktiv, sobald der Wrapperparameter
über 0,5 steht.

**Station 4 — was der geerbte Zweig tut.**
`…/juce_audio_processors/processors/juce_AudioProcessor.cpp:591-606`:

```
template <typename floatType>
void AudioProcessor::processBypassed (AudioBuffer<floatType>& buffer, MidiBuffer&)
{
    …
    jassert (getLatencySamples() == 0);

    for (int ch = getMainBusNumInputChannels(); ch < getTotalNumOutputChannels(); ++ch)
        buffer.clear (ch, 0, buffer.getNumSamples());
}
```

Bei gleichem Ein- und Ausgangsbus (der einzige erlaubte Fall,
`SondeProcessor.cpp:345-367`) läuft die Schleife nullmal: der Puffer geht **unverändert**
durch. Der `jassert` hält, weil Probeeq 0 Latenz meldet (`SondeProcessor.cpp:236`) und
`getTailLengthSeconds()` 0,0 ist (`SondeProcessor.h:174`).

**Station 5 — was dadurch NICHT läuft.** `processBlock` wird für diesen Block gar nicht
gerufen. Damit entfällt:

- `dspKern->verarbeite` (`SondeProcessor.cpp:432`) — Filterzustände, Rampen und Crossfades
  bleiben eingefroren, kein Blockrand, keine Übernahme;
- `analyseQueue.veroeffentliche` (`:466`, `:475`) — **Probeeq misst nichts**;
- `verarbeiteteSamples.fetch_add` (`:433`) — und damit steht die Ruhegrenze der
  Automationsepoche (`:1468-1473`), die in verarbeiteten Samples zählt.

Station 5 ist in der Befundzeile nicht enthalten. Für ein Produkt, dessen Probeeq-Rolle
„misst passiv auf dem Bus und beliefert Gen" ist (`CLAUDE.md:22-24`), ist der stille
Messausfall die schwerere Folge als der Sprung im Klang.

**Station 6 — der eigene Bypass sieht anders aus, aber anders als die Karte sagt.**
`eq-copilot/plugin/dsp/DspProgramm.cpp:218-223`:

```
bool rampenKompatibel (const DspProgramm& alt, const DspProgramm& neu) noexcept
{
    // Beide Programme muessen den Pfad OEFFNEN: ein Passthrough hat keine
    // Filter, deren Zustand wandern koennte, und der Weg in ihn hinein oder
    // aus ihm heraus ist `blockrand` mit Crossfade (R2, M-03, M-04, M-06).
    if (! alt.eqEngagiert || ! neu.eqEngagiert || alt.hardBypass || neu.hardBypass) return false;
```

und `eq-copilot/plugin/dsp/DspKern.cpp:328-330`:

```
    const bool pfadGleich = m.gueltig && m.eqEngagiert == prog.eqEngagiert && m.hardBypass == prog.hardBypass
                         && m.samplerate == prog.samplerate && m.monoBassAn == (prog.monoBassHz > 0.0);
    prog.pfadKennung = pfadGleich ? m.pfadKennung : naechsteKennung();
```

Ein Wechsel von `v1.global.bypass` ändert `hardBypass`, also die **Pfadkennung**. Im
Blockrand (`DspKern.cpp:569-610`) ist damit weder `nurRampen` noch `pfadGleich` wahr:
**kein** Slot überträgt seinen Zustand, alle acht starten kalt. Der interne Bypass ist
weich (Crossfade über `kFadeSamples`, `:612-616`) — aber er **behält den inneren Zustand
nicht**. Das ist auch so entschieden: E-8, `docs/beweise/SONDE-015.md:3400`, und M-07,
`:1040`.

**Erreichbarkeit: Stufe 1.** Der Bypassknopf am FL-Mixerslot.

### c) Zusage wörtlich

**Keine Zusage zum Hostbypass gefunden.** Die drei einschlägigen Stellen reden alle vom
**internen** Hard-Bypass:

- SONDE-015 M-06, `docs/beweise/SONDE-015.md:1039`: „Auch der Hard-Bypass wechselt am
  **Blockrand mit Crossfade** (`wechsel: blockrand` im Vertrag), nicht abrupt. Nach dem Fade
  gilt M-05 bitgenau."
- SONDE-015 M-05, mitgeführt in A16 (`tools/beweise.ps1:688`): „eq_enabled an mit bypass an
  bitgleich trotz hoerbarem Band dahinter".
- Entwurf `docs/FL-Nakama-Sonden-Design-Entwurf.md:3018`: „Ein neutraler Active-State
  besitzt einen expliziten Hard-Bypass vor M/S-Matrix und Filterbank."; `:3099`: „Passive
  Probe **und neutraler Hard-Bypass der Active Probe** bestehen bitgenaue Nulltests".

Eine Suche nach „Hostbypass", „Host-Bypass" oder „Slot-Bypass" im Entwurf und in
`docs/beweise/SONDE-015.md` liefert **nichts**. Auch zum Messausfall (Station 5) schweigen
die Quellen: `CLAUDE.md:22-24` beschreibt die Rolle, nennt aber keine Regel für einen vom
Host stillgelegten Messpunkt.

Die Quellen schweigen damit ausdrücklich zu beidem — Übergang und Messung.

### d) Heutige Abdeckung

| Prüfstelle | Was sie misst | Warum sie den Befund nicht fängt |
|---|---|---|
| B6 `DspGoldenTestMain.cpp:2985-3029`, Fälle `bypasswechsel_ist_klickfrei (M-06)` und `bypasswechsel_endet_bitgleich`; Bein `tools/beweise.ps1:723` | den Wechsel von `v1.global.bypass` am Kern | anderer Einstieg: der Kern wird im Hostbypass gar nicht gerufen |
| B6 `DspGoldenTestMain.cpp:2791-2815`, `hard_bypass_schreibt_nicht (M-05)` | Bitgleichheit im internen Bypass | dito |
| A16 `SondeNullTestMain.cpp:585-600`, `eq_an_bypass_an_ist_bitidentisch` (`tools/beweise.ps1:688`) | interner Bypass am Prozessor | ruft `processBlock`, nie `processBlockBypassed` |
| Paket E (`roh/phase-16-e-last-lebenslauf.md:53`) | `processBlockBypassed()` bei aktivem Zustand ergibt einen bitgleichen Puffer | ausgeführt, aber ausdrücklich „ein Komponentenbeleg zum Pfad von T3-01-09, keine Auflösung seiner noch fehlenden Übergangs-/Historienregel" |

Kein Bein am HEAD ruft `processBlockBypassed` und misst danach den Wiedereinstieg oder die
Analyseframes.

### e) Urteil: BESTÄTIGT als LÜCKE, mit zwei Präzisierungen

Die Quellkette ist am HEAD vollständig und eindeutig; die Wirkung ist unbestritten und von
Paket E an der echten Klasse ausgeführt. Was fehlt, ist eine Zusage, die sie bricht (c) —
also eine Lücke mit Produktentscheid, keine Regelverletzung. Das deckt sich mit der
Phase-10-Herabstufung (`BEFUNDE.md:503`).

Zwei Präzisierungen, beide neu:

1. **Probeeq misst im Hostbypass nicht mehr** (Station 5). Der User drückt einen
   Bypassknopf und erwartet, dass der EQ nichts tut; dass damit auch die Messung aufhört
   und Gens Quelle veraltet, ist eine zweite, ungeschriebene Folge desselben Handgriffs.
2. **Der interne Bypass erhält den Zustand ebenfalls nicht** (Station 6). Das ist für die
   Karte U48 entscheidend und wird dort behandelt.

### f) Kategorie und Schwere

**LÜCKE, medium.** Ein hörbarer Sprung bei einem gewöhnlichen Handgriff, plus ein stiller
Messausfall — aber ohne gebrochene Zusage. Der Produktentscheid ist offen und liegt als
Karte U48 in `docs/plan/fragen.json:52-62`; das Gate S25k verlangt sie ausdrücklich
(„Produktentscheide als Karten: Bypass- und Reset-Historie").

### g) Kleinster Fix

**Ort:** `eq-copilot/plugin/sonde/SondeProcessor.h` und
`eq-copilot/plugin/src/PluginProcessor.h` (je eine Override), sowie
`eq-copilot/plugin/dsp/DspKern.h/.cpp` für den Eintritt.

**Idee, technisch entschieden (die Produktwirkung entscheidet U48).** Es gibt zwei Wege, und
ich wähle den zweiten:

1. `getBypassParameter()` auf Nakamas eigenen `v1.global.bypass` zeigen lassen. Dann bedient
   FLs Slotknopf denselben Parameter — mit Crossfade, mit Historie, ohne Messausfall. Der
   Preis ist schwer: der Parameter ist persistent, revisionsfähig und Teil des
   `state_hash` (`NakamaParameter.cpp:49`). FLs Slotbypass würde damit den gespeicherten
   Zustand verändern und Host-Dirty melden — ein Handgriff, den der User als
   Ansichtsschalter versteht, schriebe ins Projekt. Das verwerfe ich.
2. `processBlockBypassed` überschreiben und darin denselben Weg wie `processBlock` fahren,
   nur mit einem **transienten** Bypasswunsch am Kern (ein Atomic neben `hoerwunsch`,
   `DspKern.h:244`), der den Pfad in denselben Übergang schickt wie M-06 und den Taps
   weiterhin Material gibt. Der bestätigte Zustand, die Revision, der Hash und der
   Undo-Ring bleiben dabei unberührt; die Messung läuft weiter. Das ist die kleinere
   Änderung und die ehrlichere: ein Hostschalter bleibt ein Hostschalter.

**Mitzuprüfende Beziehung:** **aktivieren↔abklingen** — Eintritt **und** Wiederanschluss
müssen dieselbe Regel haben (das ist genau das, was heute fehlt); **verbinden↔trennen** —
läuft die Messung im Bypass weiter, muss sie das ehrlich tun: entweder sie misst den
durchgereichten Eingang (dann ist der Messwert gültig, aber der Kontext „EQ aus" muss
mitreisen), oder sie meldet einen ausdrücklichen Zustand an Gen statt still zu veralten.
Diese Wahl ist ein Produktentscheid und gehört in die U48-Erweiterung (Abschnitt „Karte
U48").

### h) Ort des Rotbeweises

**Bein A16** (`EqCopProbeeqNullTest`, Behauptungszeile `tools/beweise.ps1:688`), Datei
`eq-copilot/plugin/tests/SondeNullTestMain.cpp`, neuer Abschnitt neben dem M-05-Fall
`:585-600` — dort liegt die vorhandene Bypassmechanik am Prozessor.

Aufbau: 48 kHz, Block 256, ein hörbares Band (Bell 1 kHz, +12 dB, Q 10) und Output-Trim
+6 dB, eingeschwungen an einem konstanten Eingang; dann drei Phasen — `processBlock`,
`processBlockBypassed` für 8 Blöcke, wieder `processBlock`. Assertionen:

- **Übergang**: der Nachbarsprung am ersten Bypass-Sample und am ersten Sample danach liegt
  innerhalb der E-31-Schranke (`docs/beweise/SONDE-015.md:3425`) — **heute ein voller
  Sprung von ×2 auf ×1**, also rot. Die Zeile, die die Zusage tragen wird, ist die neue
  Override; **vor** dem Fix fällt der Rotbeweis an der Weiche
  `juce_audio_plugin_client_VST3.cpp:3906`, die heute den Kern umgeht — der Test muss
  deshalb `processBlockBypassed` direkt rufen, nicht den Wrapper nachbauen.
- **Historie**: der Ausgang nach dem Bypass gleicht über 100 Perioden dem eines
  Referenzprozessors ohne Bypass innerhalb 0,5 dB Perioden-RMS — heute startet der Kern
  nicht kalt, sondern setzt eingefroren fort; nach einem Fix nach Weg 2 muss die gewählte
  Regel messbar sein (fortsetzen **oder** kalt, je nach U48).
- **Messung**: über die acht Bypassblöcke steigt der Zähler der angenommenen Analyseframes
  **um acht** — heute um null. Das ist der Rotbeweis für Station 5 und der wichtigere.
- **Gegenfall**: der interne `v1.global.bypass` bleibt bitgleich (M-05,
  `SondeNullTestMain.cpp:585-600` hält heute und muss halten bleiben);
- **Gegenfall**: Latenz bleibt 0 und Tail 0,0 (`jassert` in
  `juce_AudioProcessor.cpp:600` und M-51).

Die Behauptungszeile `tools/beweise.ps1:688` nennt heute den Hostbypass nicht; sie muss ihn
als eigenen Eintritt benennen, sonst misst das Bein etwas, das das Manifest nicht behauptet.

---

## 8. T3-01-10 — Gens Host-Reset beendet eine aktive Hörmarkierung nicht (W06)

*Befundzeile: `BEFUNDE.md:352` · Skeptikerzeile Phase 10: `:504` · Kartenentwurf PF11-02:
`:217-221` · Arbeitsübersicht W06: `:809` · Rohbelege: `roh/phase-01-echtzeit.md:101-105`,
`roh/phase-16-e-last-lebenslauf.md:49`, `:57`*

### a) Behauptung des Audits

Gens eigener Host-Reset bleibt ohne Behandlung der Hör-Markierung; deren Filter-, Puls- und
Fadehistorie bleibt bei geerbtem `reset()` bestehen (`BEFUNDE.md:352`).

### b) Quellkette am HEAD

**Station 1 — der Wrapper ruft `reset()` beim Stop.**
`…/juce_audio_plugin_client_VST3.cpp:3612-3616`:

```
    tresult PLUGIN_API setProcessing (TBool state) override
    {
        if (! state)
            getPluginInstance().reset();
```

**Station 2 — Gen erbt die leere Basis.** `eq-copilot/plugin/src/PluginProcessor.h:111-114`:

```
    void prepareToPlay (double samplerate, int blockSize) override;
    void releaseResources() override {}

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
```

Kein `reset()` in der ganzen Klasse (repoweite Suche). Gerufen wird daher
`…/juce_audio_processors/processors/juce_AudioProcessor.cpp:589`:
`void AudioProcessor::reset() {}`.

Zum Vergleich: Probeeq **hat** den Eintritt, `SondeProcessor.h:142-147` und
`SondeProcessor.cpp:315-343`, und der Kommentar `:317-319` nennt ausdrücklich die
Wrapperzeile aus Station 1.

**Station 3 — was die Markierung an Historie hält.**
`eq-copilot/plugin/src/HoerMarkierung.h:791-794`:

```
    Zust zust[2][kMarkierungMaxSektionen] {};
    …
    double fade = 0.0;
    int pulsPos = 0;
```

`Zust` sind die Biquad-Zustände (`:700`), `fade` die laufende Blende, `pulsPos` die
Hüllkurvenposition. Zurückgesetzt werden sie nur in `resetZustaende()` (`:753-759`) und
`hartAus()` (`:760-765`) — beides **privat**.

**Station 4 — der einzige öffentliche Weg dorthin ist `prepareToPlay`.**
`HoerMarkierung.h:297-320`:

```
    Schritt vorbereiten (int maxBlock)
    {
        Schritt uebergang;
        if (warHoerbar)
        {
            uebergang.endete = true;
            uebergang.dauerSamples = hoerbareSamples;
        }
        wetKapazitaet = std::max (maxBlock, 16);
        wet.calloc ((size_t) wetKapazitaet * 2);
        hartAus();
```

Gerufen wird es aus `eq-copilot/plugin/src/prozessor/Hostbruecke.cpp:75`
(`markierung.vorbereiten (maxBlock)`), also aus `EqCopilotProcessor::prepareToPlay`
(`:34`).

**Station 5 — und `prepareToPlay` erledigt dort viel mehr als das Leeren.**
`Hostbruecke.cpp:74-78`:

```
    const auto uebergangA = markierung.setzeSamplerate (sichereSamplerate);
    const auto uebergangB = markierung.vorbereiten (maxBlock);
    const bool markerAbgebrochen = uebergangA.endete || uebergangB.endete;
    const std::uint64_t abgebrocheneDauer =
        std::max (uebergangA.dauerSamples, uebergangB.dauerSamples);
```

und `:88-108` begründet, warum das nötig ist:

```
    // 🔑 NAK-180 N-10: der Marker ist hart aus, sein `end` kommt NIE.
    //
    // `markierung.vorbereiten()` und `setzeSamplerate()` setzen `warHoerbar`
    // zurueck und loeschen den Fade, ohne den faelligen Uebergang zu melden —
    // der Audiothread erzeugt fuer dieses Intervall also kein `endete` mehr.
    // Ohne diese Markierung bliebe das Begin beim Broker fuer immer offen und
    // die Sitzung dauerhaft gesperrt.
```

**Station 6 — wozu der Übergang gebraucht wird.**
`eq-copilot/plugin/src/PluginProcessor.cpp:980-1020` macht aus `Schritt.begann` und
`Schritt.endete` die zwei P0-Interventionsereignisse; `:1011-1019` setzt das
`project_sample_end` aus `schritt.endeOffsetSamples`.

**Damit ist der eigentliche Verlust benannt: nicht die Audiohistorie, sondern das offene
Interventionsintervall.** `reset()` beendet die Verarbeitung, ohne ein `end` zu erzeugen —
und der Pfad, der dieses Problem für `prepareToPlay` bereits gelöst hat
(`Hostbruecke.cpp:88-142`), wird nicht betreten. Die Befundzeile nennt „Filter-, Puls- und
Fadehistorie"; das ist die kleinere Hälfte.

**Station 7 — `releaseResources` ist dieselbe Lücke.** `PluginProcessor.h:112`
`void releaseResources() override {}` — auch der zweite Rückweg tut nichts. Von den drei
Hosteintritten, die NAK-283 R-283-3 als Trio führt (Probeeq: `prepareToPlay`,
`releaseResources`, `reset`; `SondeProcessor.cpp:301-307`), bedient Gen **einen**.

**Erreichbarkeit: Stufe 1.** Ein Markierungsauftrag läuft (`markierungEinreichen`,
in `MarkierungTestMain.cpp:271` und im Produkt), der User drückt Stop.

### c) Zusage wörtlich

**Keine Gen-Reset-Zusage gefunden.** Gesucht in `docs/beweise/SONDE-013.md` (Markierung und
Interventionsring), `docs/beweise/SONDE-015.md` (M-30 bis M-34 sind Mix und M/S, nicht
Reset), im Entwurf und in `docs/offene-punkte.md`. Das Gegenstück für **Probeeq** existiert
und ist datiert: NAK-283 F05, ausgeführt als A16-Regressionswache
(`eq-copilot/plugin/tests/SondeNullTestMain.cpp:427-441`,
`nulltest_bleibt_bitidentisch_mit_host_reset (M-34)`) und als B7-Fall
(`tools/beweise.ps1:728`: „reset() beendet die Audiohistorie …"). Diese Zusage gilt
ausdrücklich für die Sonde.

Die **nächstgelegene** Zusage, gegen die der Befund laufen könnte, ist §34.2 des Entwurfs
über die Quarantäne des gefärbten Audios; sie verlangt einen geschlossenen Intervallrand,
sagt aber nichts über den Hostreset. Der Kommentar `Hostbruecke.cpp:93-94` formuliert die
dahinterliegende Regel als Tatsache („Ohne diese Markierung bliebe das Begin beim Broker
fuer immer offen und die Sitzung dauerhaft gesperrt") — das ist eine Begründung im Code,
keine Zusage in einer Quelle.

Die Quellen schweigen. Das steht hier ausdrücklich.

### d) Heutige Abdeckung

| Prüfstelle | Was sie misst | Warum sie den Befund nicht fängt |
|---|---|---|
| A3 `MarkierungTestMain.cpp` (Bein `tools/beweise.ps1:509`) | Markierung am echten `EqCopilotProcessor` (`:263`, `:271`, `:322`, `:451`), Oversize-Riegel, `prepareToPlay`-Wege (`:266`, `:325`, `:406`, `:424`, `:454`, `:531`) | **kein einziger `reset()`-Aufruf** in der Datei. Repoweite Suche über `eq-copilot/plugin/tests/*.cpp`: kein Test ruft `AudioProcessor::reset()` auf dem Gen-Prozessor. |
| A16 `SondeNullTestMain.cpp:427-474` (`tools/beweise.ps1:688`) | `reset()` vor jedem dritten Block, bitgleich, Zähler bewegen sich nicht | das ist der **Probeeq**-Prozessor (`p->reset()` in `:464`, `:466`), nicht Gen |
| Paket E (`roh/phase-16-e-last-lebenslauf.md:49`) | „Acht aktive Bänder einschwingen → `reset()` → Stille: Ausgang exakt 0" | wieder Probeeq. `:57` sagt es selbst: „Gens aktive Hörmarkierung über den geerbten Hostreset bleibt T3-01-10; der hier geprüfte Sonde-Reset ist kein Gegenbeweis zu dieser anderen Klasse." |

### e) Urteil: BESTÄTIGT als LÜCKE, mit einer Präzisierung

Die Quellkette ist am HEAD vollständig; `PluginProcessor.{h,cpp}` ist seit dem Auditstand
nur an der v2-Pipenamenwahl verändert (NAK-309), `HoerMarkierung.h` gar nicht. Eine
Gen-Reset-Zusage gibt es nicht (c). Damit bleibt es eine Lücke mit Produktentscheid.

**Präzisierung:** der Kern des Befundes ist das **offene Interventionsintervall**
(Stationen 5 und 6), nicht die Filterhistorie. Der Code kennt das Problem bereits und hat
es für `prepareToPlay` sorgfältig gelöst (`Hostbruecke.cpp:88-142`) — das ist zugleich der
stärkste Beleg dafür, dass hier etwas fehlt und nicht etwas bewusst offen gelassen wurde.

**Zweite Präzisierung:** `releaseResources` (Station 7) hat dieselbe Lücke. Sie ist keine
eigene ID, gehört aber in denselben Änderungssatz — das verlangt `CLAUDE.md:84-86`
ausdrücklich („starten↔stoppen").

### f) Kategorie und Schwere

**LÜCKE, medium.** Ohne Zusage, aber mit zwei konkreten Folgen: ein Markierungsklang, der
nach Stop/Start weiterläuft, und ein Intervall, das beim Broker offen bleibt und die
Evidenz der Sitzung dauerhaft als gefärbt führt. Der zweite Punkt hebt die Schwere über
„low": eine Sitzung, deren Quarantäne nie endet, liefert Gen keine verwertbaren Befunde
mehr. Gegen „high" spricht, dass es kein gemessener Fall ist und dass der Wiederanschluss
ohnehin einen Produktentscheid braucht.

### g) Kleinster Fix

**Ort:** `eq-copilot/plugin/src/PluginProcessor.h` (neue Override `void reset() override;`
neben `:111-114`) und `eq-copilot/plugin/src/prozessor/Hostbruecke.cpp` (Implementierung
neben `prepareToPlay`, `:34`).

**Idee.** `reset()` bekommt denselben Abschlussweg wie `prepareToPlay`, aber **ohne** die
Allokation: es ruft den privaten Abbruchpfad der Markierung über einen neuen öffentlichen
Einstieg (`Schritt beendeAuftrag()`, der `hartAus()` ruft und den fälligen Übergang wie
`vorbereiten` in `:299-303` zurückgibt) und führt danach genau den Block
`Hostbruecke.cpp:97-142` aus, der das synthetische `end` mit `project_sample_end: null`
bildet. `wet.calloc` und `versuchTrocken.assign` bleiben aus — `reset` allokiert nicht. Der
Oversize-Riegel (`HoerMarkierung.h:444`) darf dabei **nicht** gelöst werden, denn der ist
ausdrücklich „bis zum nächsten `prepareToPlay`" definiert (`:308-312`).

`releaseResources` (`PluginProcessor.h:112`) bekommt denselben Aufruf; seine Blockpuffer
dürfen dabei zusätzlich freigegeben werden.

**Was der Fix NICHT entscheidet:** ob nach einem Resume der Markierungs**auftrag** weiter
gelten soll. Das ist die Produktfrage (PF11-02, `BEFUNDE.md:217-221`), und sie hat heute
**keine Karte** (siehe Abschnitt „Karte U48"). Bis zur Antwort baue ich den technisch
sicheren Weg: Audiohistorie und Intervall enden, der Auftrag bleibt eingereicht und beginnt
beim nächsten erlaubten Block neu — das ist derselbe Zustand, den `prepareToPlay` heute
herstellt, also keine neue Semantik.

**Mitzuprüfende Beziehung:** **starten↔stoppen** — die drei Hosteintritte
`prepareToPlay`, `releaseResources` und `reset` müssen bei Gen dasselbe Trio bilden wie bei
Probeeq (`SondeProcessor.cpp:301-307`); **öffnen↔schließen** — jedes gesendete `begin`
braucht genau ein `end`, und der Überlaufpfad (`Hostbruecke.cpp:104-108`,
`interventionsRingUeberlauf`) muss fail-closed bleiben; **aktivieren↔abklingen** — der Fade
endet, ohne dass `hoerbarAtomic` zu früh auf false fällt (der Kommentar
`HoerMarkierung.h:315-318` erklärt, warum `vorbereiten` das Atomic bewusst nicht anfasst).

### h) Ort des Rotbeweises

**Bein A3** (`EqCopMarkierungTest`, Behauptungszeile `tools/beweise.ps1:509`), Datei
`eq-copilot/plugin/tests/MarkierungTestMain.cpp`, neuer Fall neben dem vorhandenen
M-36-Oversize-Fall bei `:449-547` — dort steht die Mechanik (Prozessor auf dem Heap,
laufender Transport, eingereichter Auftrag, Riegel bis `prepareToPlay`), die dieser Fall
spiegelt.

Aufbau: `EqCopilotProcessor` auf dem Heap (`:451`, MSVC-Stack, NAK-175), `prepareToPlay`
(`:454`), laufender Transport (`LaufenderTransport`, `:119`, hier `:456`),
`markierungEinreichen` (`:458`) mit einem
hörbaren Auftrag, Blöcke fahren bis `hoerbar()` wahr ist; dann **`p.reset()`**; danach
Blöcke ohne neues `prepareToPlay`. Assertionen:

- der Ausgang des ersten Blocks nach `reset()` ist **bitgleich zum Eingang** — heute
  klingt die Markierung weiter, also rot. Die Zeile, die die Zusage tragen wird, ist die
  neue Override; **vor** dem Fix fällt der Rotbeweis an `PluginProcessor.h:111-114`, wo
  `reset` fehlt — der Test misst die Wirkung, nicht die Zeile, und das ist hier richtig,
  weil es die fehlende Zeile ist.
- der Interventionsring enthält nach dem `reset()` genau **ein** `end` mit
  `project_sample_end` null und der gezählten Hördauer — heute keines. Das ist der
  wichtigere Teil und die Entsprechung zu `Hostbruecke.cpp:97-142`.
- `interventionsRingUeberlauf` bleibt false (kein zweites offenes Intervall);
- **Gegenfall 1**: derselbe Ablauf mit `prepareToPlay` statt `reset()` erzeugt genau dasselbe
  `end` (hält heute, misst dass der Fix den vorhandenen Weg wiederverwendet statt einen
  zweiten zu bauen);
- **Gegenfall 2**: `reset()` **ohne** aktive Markierung erzeugt **kein** Ereignis und lässt
  die Statebytes bytegleich;
- **Gegenfall 3**: der Oversize-Riegel bleibt nach `reset()` gesetzt und löst sich erst beim
  nächsten `prepareToPlay` (`MarkierungTestMain.cpp:529-546` misst das heute für den
  `prepareToPlay`-Weg, Fall „M-36: nach prepareToPlay ist sie wieder moeglich");
- **Gegenfall 4**: dasselbe für `releaseResources()` (Station 7).

Die Behauptungszeile `tools/beweise.ps1:509` nennt den Hostreset heute nicht; sie muss ihn
mit dem Intervallabschluss ausdrücklich nennen.

---

## Gemeinsame Ursachen

**Vier Bündel, drei getrennte Änderungssätze, eine Reihenfolge.**

**1. W02 (T3-05-01, T3-05-02) teilen eine Wurzel: der Ladestart kennt seine Nachbarn
nicht.** `setStateInformation` (`SondeProcessor.cpp:1051-1106`) setzt den bestätigten
Zustand und die Klassifikation, lässt aber zwei Dinge unberührt, die danach weiter wirken:
die Prozessor-Mailbox (T3-05-01) und die Tatsache, dass `nurLesen` für den Automationsweg
keine Schranke ist (T3-05-02). Beide Male ist die Ursache dieselbe: **`nurLesen` und die
Ladegeneration sind Eigenschaften des Standes, aber der Automationspfad kennt nur den
Transaktionskern.** Die Fixe sind trotzdem getrennt — der eine quittiert einen Zähler
(1g), der andere setzt ein Flag im Kern (2g) —, gehören aber in **einen** Änderungssatz
und **eine** Regression, weil beide an `ladestart` hängen und eine Reparatur ohne die
andere einen halben Ladestart erzeugt.

**2. W05 (T3-01-02, T3-01-03, T3-01-05) teilen die Datei und den Thread, aber nicht den
Vertrag.** Die Skeptikerzeile sagt das bereits für die Hostparameterseite
(`roh/phase-10-skeptiker.md:122`: „T3-01-05, T3-05-01 und T3-05-02 teilen den
Hostparameterkanal, aber nicht den verletzten Vertrag"), und die Quellenlektüre bestätigt
es und erweitert es:

- **T3-01-02** ist eine Sperre, die JUCE nimmt, bevor Nakamas Code beginnt — Fix in der
  Parameteranbindung;
- **T3-01-03** ist eine Sperre, die Nakamas Code selbst nimmt — Fix in `setNonRealtime`;
- **T3-01-05** ist gar keine Sperre, sondern ein Zeitpunkt — Fix im Taktbezug des Workers.

Eine pauschale „Hostkante lockfrei machen"-Änderung träfe alle drei und keinen richtig.
**Getrennt halten.** Gemeinsam ist ihnen nur die Messlücke: die RT-Wache beginnt bei
`DspKern.cpp:1059`, und die Behauptungszeile `tools/beweise.ps1:723` behauptet mehr. Diese
Messlücke gehört **vor** alle drei (siehe Reihenfolge).

**3. W06 (T3-01-03, T3-01-04) sind ein Fix, obwohl sie in zwei Arbeitsbündeln stehen.**
Das ist das wichtigste Ergebnis dieser Lektüre. `setNonRealtime` (`SondeProcessor.cpp:1321-1328`)
ist zugleich der Ort der Sperre (W05) und der Ort der zu späten Vorschaubeendigung (W06).
Ein Fix, der nur die Sperre entfernt und die Vorschaubeendigung an den Worker gibt,
**verschärft T3-01-04** — der Worker zieht erst in bis zu 5 ms, also weit nach dem ersten
Offline-Sample. Nur die kombinierte Lösung aus 4g und 6g trägt: Wechselerkennung per
Atomic, Hörmatrix hart statt geblendet, Candidate-Bank über `beendeCandidate` aus dem
Callback, Buchhaltung beim nächsten Takt. **T3-01-03 und T3-01-04 gehören in einen
Änderungssatz und eine Regression.**

**4. W06 (T3-01-09, T3-01-10) teilen die Produktfrage, nicht den Code.** Beide sind
Hostlebenslauf-Eintritte ohne Regel, beide brauchen denselben Entscheid („was überlebt
welchen Hostübergang", `roh/phase-10-skeptiker.md:128`, P10-01). Der Code ist
unterschiedlich: einmal eine fehlende Bypass-Override an beiden Klassen, einmal eine
fehlende Reset-Override an Gen. **Getrennt bauen, gemeinsam entscheiden.**

**Was ausdrücklich NICHT zusammengehört.** T3-01-04 und T3-01-05 sehen verwandt aus (beide
Offline, beide M-120), sind es aber nicht: der eine ist ein Zustandsübergang am
Renderanfang, der andere ein Zeitbezug über den ganzen Render. Eine Reparatur des einen
lässt den anderen unberührt.

**Berührung mit den IDs aus Teil B (W09, W22, W23, W24).** Ich habe die vier Bündel gelesen,
soweit sie meine acht IDs berühren:

- **W09 (T3-04-01, T3-04-03: Provider nach 2-s-Detach, Messpunktpanel überlebt Prozessor)**
  berührt T3-01-10 an der Lebensdauerfrage, aber an anderen Objekten: dort geht es um
  Editor- und Providerlebenszeiten auf dem Nachrichtenthread, hier um Audiohistorie und
  Interventionsintervall auf dem Audiothread. **Keine gemeinsame Änderung.** Berührung
  besteht in der Regression: Paket E hat Editorzyklen und Prozessorabbau zusammen gefahren
  (`roh/phase-16-e-last-lebenslauf.md:61-67`); ein Folgebein für beide Bündel kann denselben
  Harnisch nutzen.
- **W22 (T3-02-06, T3-07-02: Rollenwechsel Main/Legacy)** berührt T3-05-01 und T3-05-02 an
  derselben Stelle: `setStateInformation` klassifiziert (`SondeProcessor.cpp:1072`
  `lebenslauf.stateRestauriert`). Wer dort die Mailboxbarriere und das `nurLesen`-Flag
  einzieht, fasst dieselbe Funktion an, die W22 für den Rollenwechsel braucht.
  **Reihenfolge beachten, nicht zusammenlegen** — W22 hat einen eigenen Produktentscheid
  (U49), W02 keinen.
- **W23 (T3-14-01: Label des vorigen Ziels)** und **W24 (T3-07-05: Quellen ab 21)** berühren
  keine meiner acht IDs; beide liegen in Gens Quellenmodell, nicht an der Audio- oder
  Statekante.

---

## Karte U48 — sie trifft nur zur Hälfte und beschreibt die andere Hälfte falsch

**Kartentext heute**, `docs/plan/fragen.json:52-62`:

> „**titel:** FLs eigener Bypass-Schalter: genauso weich wie Nakamas Schalter?
> **was:** FL hat einen eigenen Bypass je Plugin-Slot. Nakamas eigener EQ-Schalter blendet
> weich über und behält den inneren Zustand. (1) Empfohlen: der FL-Bypass verhält sich
> genauso — weich, Zustand bleibt erhalten, danach geht es nahtlos weiter. (2) Der
> FL-Bypass schaltet hart um.
> **warum:** Audit-Befunde T3-01-09 **und T3-01-10** (Kartenentwurf PF11-01) …"

**Drei Befunde an dieser Karte.**

**Erstens: die Prämisse ist am HEAD falsch.** „Nakamas eigener EQ-Schalter … **behält den
inneren Zustand**" trifft nicht zu. Ein Wechsel von `v1.global.bypass` ändert
`hardBypass`, damit die Pfadkennung (`eq-copilot/plugin/dsp/DspKern.cpp:328-330`), damit
weder `nurRampen` noch `pfadGleich` (`:569-576`) — **alle acht Slots starten kalt**. Das
ist auch so entschieden: E-8, `docs/beweise/SONDE-015.md:3400` („Nein — sie startet kalt,
alle Zustände auf 0") und M-07, `:1040`. Der User wird also gebeten, „genauso wie Nakamas
Schalter" zu wählen, und die Beschreibung dieses Schalters stimmt nicht. Option (1) ist in
ihrer heutigen Formulierung nicht baubar, ohne zugleich E-8 umzuwerfen.

**Zweitens: die Karte deckt T3-01-10 nicht ab, obwohl sie ihn nennt.** Ihr `was`-Text
spricht ausschließlich vom Bypass. Der Kartenentwurf des Audits hat dafür eine **eigene**
Karte vorgesehen — PF11-02, `BEFUNDE.md:217-221`: „Was überlebt reset: volatile
Audiohistorie, Markierungsauftrag, Interventionszeitraum?" Eine Durchsicht aller 31 Karten
in `docs/plan/fragen.json` (Status `offen`, `geparkt`, `zugesagt`) ergibt: **es gibt keine
Karte zum Gen-Reset.** Die Registerzeile `docs/offene-punkte.md:316` führt „Bypass- und
Reset-Historie (W06)" als **einen** Produktentscheid; tatsächlich sind es zwei Fragen an
zwei verschiedenen Klassen.

**Drittens: die hörbar wichtigste Folge fehlt.** Im Hostbypass **misst Probeeq nicht mehr**
(Abschnitt 7, Station 5). Für einen Musiker ist das die überraschendere Wirkung: Gens
Quellenliste veraltet, weil er einen Bypassknopf gedrückt hat. Die Karte erwähnt es nicht.

### Vorschlag: U48 berichtigen und U56 neu anlegen

**U48, neuer Wortlaut** (Alltagssprache, keine Oberfläche, keine Technik):

> **titel:** FLs eigener Bypass-Schalter: was soll er mit Klang und Messung machen?
>
> **was:** FL hat an jedem Mixer-Slot einen eigenen Bypass-Knopf, getrennt von Nakamas
> eigenem EQ-Schalter. Heute schaltet FLs Knopf hart um: der EQ verschwindet von einem
> Sample zum nächsten, und Probeeq hört auf zu messen — in Gen veraltet diese Quelle,
> solange der Knopf gedrückt ist. Nakamas eigener Schalter blendet dagegen weich über
> (etwa 5 ms), fängt danach aber ebenfalls klanglich neu an: die Filter starten kalt, ein
> langer Nachhall im EQ ist weg.
>
> (1) **Empfohlen:** FLs Knopf blendet genauso weich über wie Nakamas eigener Schalter, und
> die Messung läuft währenddessen weiter — Gen zeigt die Spur weiter an, erkennbar als „EQ
> aus".
> (2) FLs Knopf bleibt hart, aber die Messung läuft weiter.
> (3) Alles bleibt wie heute: hart, und die Messung pausiert.
>
> **warum:** Audit-Befund T3-01-09. Ohne Antwort baut der Dirigent Weg 1 (Planschritt S25k,
> Ticket NAK-312). Ob die Filter beim Zurückschalten kalt starten, ist in beiden Fällen
> gleich und schon entschieden (E-8) — das ist nicht Teil dieser Frage.

**U56, neue Karte** (für T3-01-10 / PF11-02):

> **titel:** Stop drücken, während eine Passage markiert klingt: was passiert damit?
>
> **was:** Du kannst dir eine Passage hörbar markieren lassen. Drückst du mitten in dieser
> Markierung Stop, passiert heute nichts Bestimmtes: die Markierung bleibt intern stehen,
> klingt beim nächsten Start weiter, und der Zeitraum, den Nakama als „durch die Markierung
> eingefärbt" führt, wird nie geschlossen — die Messwerte dieser Sitzung bleiben dann
> dauerhaft als unbrauchbar markiert.
>
> (1) **Empfohlen:** Stop beendet die Markierung sauber: sie verstummt, der eingefärbte
> Zeitraum wird geschlossen, und beim nächsten Start beginnt sie von vorn, weil der Auftrag
> noch steht.
> (2) Stop beendet die Markierung **und** den Auftrag: nach dem Start ist nichts mehr
> markiert, bis du es neu auslöst.
>
> **warum:** Audit-Befund T3-01-10 (Kartenentwurf PF11-02). Ohne Antwort baut der Dirigent
> Weg 1 (Planschritt S25k, Ticket NAK-312).
>
> **wo:** `docs/audits/2026-09-15-tiefenaudit/BEFUNDE.md` T3-01-10, PF11-02,
> Arbeitsübersicht W06; `docs/offene-punkte.md` NAK-312.

**Was kein Produktentscheid ist und deshalb nicht auf eine Karte gehört:** ob Probeeq im
Bypass den durchgereichten Eingang misst oder einen ausdrücklichen Zustand meldet
(Implementierungsfrage zu Option 1 und 2), ob `getBypassParameter` oder
`processBlockBypassed` der Eintritt ist (7g), und ob der Kaltstart der Filter bleibt (E-8
ist entschieden).

---

## Reihenfolge

**Vor allem anderen: V0 und die Messlücke der Wache.**

- **Etappe 0a — die Wache sehen lassen, was sie behauptet.** `RtWache::Bereich` von
  `DspKern.cpp:1059` auf `SondeProcessor::processBlock` (`:369`) heben und die
  Behauptungszeilen `tools/beweise.ps1:723` und `:688` auf das beschränken, was sie messen.
  **Das muss vor T3-01-02, T3-01-03 und T3-01-05 stehen**, sonst beglaubigt ein zu enger
  Zähler jede folgende Reparatur. Verhaltensneutral, eigenes kleines Ticket nach Muster
  S19b. Das ist derselbe Gedanke, den NAK-311 für T3-09-02 durchgesetzt hat: **erst das
  Messgerät, dann das Produkt.**
- **Etappe 0b — eine Hostmessung, kein Fix.** Im Laufzeit-Arm protokollieren, ob FL vor
  einem Offline-Render `prepareToPlay`/`releaseResources` ruft (Abschnitt 6, Station 5).
  Das entscheidet die Erreichbarkeitsstufe von T3-01-04 und gehört **vor** dessen
  Einordnung, nicht vor dessen Reparatur (die Arbeit ist in beiden Fällen dieselbe).
  Voraussetzung ist V1 (`BEFUNDE.md:803`).

**Danach, in dieser Reihenfolge:**

1. **Etappe 1 — W02 zusammen: T3-05-01 und T3-05-02.** Beide hängen an `ladestart`
   (Gemeinsame Ursachen 1). Ein Änderungssatz, eine Regression an B7, Gegenfälle für den
   Rückweg. Beginnt hier, weil es der einzige Befund mit `high` und ohne offenen
   Produktentscheid ist — nichts blockiert ihn.
2. **Etappe 2 — T3-01-05.** Der zweite `high`-Befund ohne Produktentscheid, und der
   einzige mit hörbarer Folge im Export. Braucht Etappe 0a als Messgrundlage nicht zwingend
   (der Rotbeweis misst Audio, keine Sperren), aber die Regression liegt im selben Bein wie
   Etappe 1.
3. **Etappe 3 — T3-01-03 und T3-01-04 zusammen** (Gemeinsame Ursachen 3). Ein
   Änderungssatz in `setNonRealtime` plus ein Atomic im Kern. Braucht Etappe 0a, weil der
   Sperren-Rotbeweis sonst nicht fallen kann. Die Einordnung von T3-01-04 wartet auf
   Etappe 0b, die Reparatur nicht.
4. **Etappe 4 — T3-01-02.** Zuletzt unter den Technikfixen: die Wirkung ist die kleinste,
   und der Umbau der Parameteranbindung berührt den Ladeweg aus Etappe 1. Er darf nicht
   vorher kommen.

**Was erst nach einer Antwort baubar ist:**

- **T3-01-09 wartet auf U48.** Der technische Weg steht (7g, Weg 2), aber ob die Messung
  im Bypass weiterläuft und ob der Übergang weich wird, sind Produktwirkungen. Vorbereiten
  lässt sich alles: die Override-Stelle, der Rotbeweis in A16 und der Zähler für
  angenommene Analyseframes sind unabhängig von der Antwort.
- **T3-01-10 wartet auf die neue Karte U56.** Der sichere Teil ist **nicht** blockiert:
  Audiohistorie beenden und das Interventionsintervall schließen ist in beiden Optionen
  gleich (8g). Offen ist nur, ob der **Auftrag** den Stop überlebt. Ich schlage vor, den
  sicheren Teil in Etappe 5 zu bauen und die Auftragsfrage als kleinen Nachzug zu führen —
  sonst bleibt ein offenes Intervall beim Broker stehen, bis der User eine Frage
  beantwortet, die damit nichts zu tun hat.
- **Die Karten selbst** gehören vor die Etappen 5 und 6 gestellt, und zwar zusammen: U48
  berichtigen und U56 anlegen ist ein Handgriff im Register, keine Bauarbeit. Produktfragen
  nur zwischen 9 und 23 Uhr.

**Nicht in dieses Ticket:** der gepinnte JUCE-VST3-Wrapperpatch aus §44.3
(`docs/FL-Nakama-Sonden-Design-Entwurf.md:3072-3082`). Er würde T3-01-02 an der Wurzel
lösen und T3-01-05 vollständig statt nur blockgebunden. Er ist ein eigener,
identitätsnaher Bau mit gepinntem Quellhash und gehört nicht in eine Audit-Nacharbeit.

---

## Nicht geprüft

- **Alle Auditmessungen als solche.** Die Werte −8,99999944679 dB (T3-05-01),
  +6,00000017258 dB (T3-05-02) und der bitgleiche `processBlockBypassed`-Puffer (T3-01-09)
  stammen aus den Phase-16-Läufen der Pakete C und E. Ich habe ihre Quellketten verifiziert
  und ihre Plausibilität nachgerechnet (10^(−9/20) und 10^(6/20)), nicht ihre Zahlen
  reproduziert. Bau und Lauf waren verboten.
- **Die Offlinerechnung in Abschnitt 5, Station 6** ist eine Herleitung aus der Quelle
  (`SondeProcessor.cpp:575-579`) und einer gemessenen Offlinegeschwindigkeit
  (`roh/phase-16-e-last-lebenslauf.md:41`), **kein** gefahrener Vergleich. Die Größenordnung
  (rund jeder zehnte Wert überlebt) ist belastbar, die genaue Zahl nicht.
- **Der Hostweg.** Kein FL-Lauf, kein VST3-Wrapper, keine Installation. Insbesondere offen:
  ob FL vor einem Offline-Render deaktiviert (entscheidet T3-01-04, Abschnitt 6e), und ob
  FLs Slotbypass tatsächlich über den Wrapperparameter läuft (die JUCE-Weiche
  `:3906` ist gelesen, die FL-Seite nicht gemessen).
- **Die JUCE-Quellen sind ein Bauartefakt.** `eq-copilot/build/_deps/juce-src/` gehört nicht
  ins Repo und verschwindet bei einem frischen Bau. Für einen **dauerhaften** Beweis dürfen
  T3-01-02, T3-01-03, T3-01-09 und T3-01-10 nicht auf diese Zeilennummern gestützt werden;
  der Rotbeweis muss die Wirkung am eigenen Prozessor messen, nicht die fremde Zeile
  zitieren. Dieselbe Einschränkung hat NAK-311 Teil A für T3-01-01 festgehalten.
- **Der Broker.** Das Interventionsintervall aus Abschnitt 8 endet auf der Brokerseite; ob
  ein offenes `begin` dort tatsächlich die Sitzung dauerhaft sperrt, steht als Begründung im
  Code (`Hostbruecke.cpp:93-94`), ist von mir aber nicht in `broker/` nachgelesen worden —
  Sicherheits- und Brokerarbeit ist delegiert (S25i, NAK-314).
- **Die sechs IDs aus Teil B** (T3-04-01, T3-04-03 in W09; T3-02-06, T3-07-02 in W22;
  T3-14-01 in W23; T3-07-05 in W24) waren nicht Gegenstand dieses Teils; ihre Berührung mit
  meinen acht ist in „Gemeinsame Ursachen" nur aus meiner Seite gelesen.
- **Die Sicherheitsbefunde T3-12-01 bis T3-12-05** sind auftragsgemäß nicht Gegenstand.

FERTIG Teil A, 7 bestätigt, 1 präzisiert, 0 widerlegt.
