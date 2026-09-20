# NAK-312 — Quellvalidierung Teil B (lesender Opus-Agent, HEAD 090d0fa1)

**Ticket:** NAK-312 · Planschritt S25k „Tiefenaudit 3 abarbeiten, Teil 4: Hostkante und
Plugin-Lebenslauf" · Etappe 0 „Quellvalidierung", **Teil B**
**HEAD:** `090d0fa10ce44fa8c6c9ac795982584c6ebcefb7` (Zweig `master`)
**Datum:** 20.09.2026 · **Modell:** Opus (lesend)
**Gegenstand (6 IDs):** T3-04-01 und T3-04-03 (W09); T3-02-06 und T3-07-02 (W22);
T3-14-01 (W23); T3-07-05 (W24). Dazu die Produktkarten **U49** und **U51**.
**Quelle der Befunde:** `docs/audits/2026-09-15-tiefenaudit/BEFUNDE.md`
(Phasenzeilen `:367`, `:398`, `:400`, `:437`, `:440`, `:588`; Skeptikerzeilen Phase 10
`:510`, `:523`, `:525`, `:534`, `:537`, Herabstufung `:53` und `:106`;
Kartenentwürfe `:225-230` (PF11-03) und `:323-328` (PF11-17);
Phase-16-Abschnitt ab `:704`; Arbeitsübersicht `:812`, `:825`, `:826`, `:827`) und die
Rohberichte `roh/phase-02-zustand.md`, `roh/phase-04-lebenslauf.md`,
`roh/phase-07-modell.md`, `roh/phase-10-skeptiker.md`, `roh/phase-11-synthese.md`,
`roh/phase-14-blindspots.md`, `roh/phase-16-c-persistenz.md`,
`roh/phase-16-e-last-lebenslauf.md`, `roh/phase-16-f-gegenpruefung-c.md`,
`roh/phase-16-abdeckung.md`.
**Gate:** Schritt S25k in `docs/plan/plan.json`; Registerzeilen `docs/offene-punkte.md:316`
(NAK-312) und `:205` (NAK-179).
**Muster:** `docs/beweise/roh/NAK-311-quellvalidierung-teil-B.md` und
`docs/beweise/roh/NAK-311-quellvalidierung-teil-A.md` (Kopf).

**Prüfform:** ausschließlich Quellenlektüre am HEAD. Kein Bau, kein Testlauf, kein Kanon,
kein Skript, kein FL Studio, kein Codex, keine Fehlerinjektion. Git nur lesend. **Jede
Zeilennummer unten ist am HEAD `090d0fa1` selbst nachgelesen**, keine aus dem Audit
übernommen; wo eine Auditzeile am HEAD verschoben ist, steht die neue Zahl.
Sicherheitsbefunde (T3-12-\*) sind nicht Gegenstand. Legacy-Bezeichner (`EQ-Copilot`,
`EqCop*`, `Eqcp`) sind kein Befund. Der Designteil ist geparkt (12.09.2026): unten steht
kein Oberflächenvorschlag, nur der nötige Handgriff oder die ehrliche Zustandsmeldung.

**Arbeitsbaum, geprüft mit `git status --short`.** *Zu Beginn des Laufs* (23:36) und
*unmittelbar vor dem Schreiben dieser Datei* (23:46) identisch: die zwei fremden untracked
Ordner `briefing-hub/` und `nimbalyst-local/` sowie die beiden Auftragsdateien
`docs/beweise/roh/NAK-312-quellvalidierung-teil-A-auftrag.txt` und
`-teil-B-auftrag.txt`, sonst nichts. HEAD beide Male `090d0fa1`. Der HEAD ist während des
Laufs **nicht** gewandert; ein Basisabgleich der Zeilennummern war deshalb nicht nötig.
Diese Datei ist die einzige Schreibaktion des Agenten. `-teil-A.md` gehört dem
Parallelagenten und wurde nicht angefasst.

**Stand des Audits gegen den HEAD.** Die Phase-16-Berichte prüften
`aff2d8188f33a9525aec1869449773bfdbe305a6`; Phase 02, 04, 07, 10, 11 und 14 liegen davor.
`git diff --stat aff2d818 090d0fa1 -- eq-copilot broker tools` berührt von den unten
zitierten Produktdateien **genau eine**: `eq-copilot/plugin/sonde/SondeProcessor.cpp`
(50 Zeilen). **Nicht** in der Diff-Liste stehen
`eq-copilot/plugin/src/PluginEditor.cpp`, `eq-copilot/plugin/src/prozessor/State.cpp`,
`eq-copilot/plugin/src/prozessor/Ipc.cpp`, `eq-copilot/plugin/state/NakamaState.cpp`,
`eq-copilot/plugin/src/HoerMarkierung.h`, `eq-copilot/plugin/core/ipc/ControlClient.cpp`
und `eq-copilot/plugin/core/ipc/TelemetryClient.cpp` — die Zeilennummern der Befunde
T3-04-03, T3-02-06, T3-07-02, T3-14-01 und T3-07-05 gelten am HEAD **unverändert**.
Der Diff in `SondeProcessor.cpp` betrifft ausschließlich `prepareToPlay` (NAK-311
R-311-3/R-311-16: Kanalzahl und Samplerateschranke) und einen Kommentar in `reset()`;
Konstruktor, Provider und Destruktor sind unberührt. Er verschiebt allerdings die
Zeilennummern der Provider-Rümpfe um rund zwanzig Zeilen — die sind unten neu gelesen.
`eq-copilot/plugin/tests/ShotTestMain.cpp` änderte sich nur im Kopfkommentar (NAK-309),
die Fallliste ist unverändert.

---

## Ergebnis in einer Zeile je ID

| ID | Urteil | Kategorie | Schwere | Ort des Rotbeweises | Produktentscheid nötig |
|---|---|---|---|---|---|
| T3-04-01 (W09) | BESTÄTIGT, zwei Präzisierungen (genau welches Mitglied zuerst stirbt; kein Bein kann den Pfad heute bauen) | DEFEKT gegen R-D2 | high (Audit: critical) | A16/B7-Familie um ein Sonde-Lebenslaufbein erweitern; vorher braucht die Sonde Testkonstruktor und Starthaken wie Gen | **nein** — R-D2 vom 25.08.2026 entscheidet die Regel |
| T3-04-03 (W09) | BESTÄTIGT, präzisiert: der Kommentar am Übergabepunkt behauptet einen Schutz, den es nicht gibt | LÜCKE | high (Audit: critical) | Bein B15/B14 mit modalem Abbau; Mutation an `PluginEditor.cpp:688` | **nein** — reine Besitz-/Lebensdauerregel |
| T3-02-06 (W22) | **PRÄZISIERT**: der Live-Rückweg erhält die Daten **nicht** vollständig; die bestätigten Quellen sind schon beim Rollenwechsel weg | LÜCKE | medium | Bein B2 `StateMigrationTestMain.cpp` neben `:2331-2334`, Gegenwache B14 | **ja** — U49 trifft, Wortlaut ist zu berichtigen |
| T3-07-02 (W22) | BESTÄTIGT, präzisiert: drei trennbare Defekte in einer Zeile; das Fensterschließen bleibt als einziger Rückweg | DEFEKT gegen CLAUDE.md:51-52 | high | Bein A3 `MarkierungTestMain.cpp` neben `:179`; Mutation an `PluginEditor.cpp:846` | **nein** — Gesetz 24.08.2026 entscheidet |
| T3-14-01 (W23) | **PRÄZISIERT**: die zitierte Zusage U02 ist **nicht** gebrochen; gebrochen ist CLAUDE.md:51-52 | DEFEKT | medium | Bein B13/B15 neben `Sonde012SourcesModelTest.cpp:460`; Mutation an `PluginEditor.cpp:934` | **nein** — Gesetz 24.08.2026 entscheidet |
| T3-07-05 (W24) | BESTÄTIGT als LÜCKE (Phase-10-Herabstufung trägt), zwei Präzisierungen | LÜCKE | medium | Bein B15 `EqCopShot` mit 21/32/64 Quellen; Mutation an `PluginEditor.cpp:958-959` | **ja** — U51 trifft, Empfehlung widerspricht dem Auditvorschlag |

---

## 1. T3-04-01 — die drei Sonde-Provider stehen außerhalb jeder Besitzschleuse (W09)

*Befundzeile: `BEFUNDE.md:398` · Skeptiker Phase 10: `BEFUNDE.md:523` · Herleitung:
`roh/phase-04-lebenslauf.md:33-48` · Threadinventar: `roh/phase-04-lebenslauf.md:95`,
`:123` · Arbeitsübersicht: `BEFUNDE.md:812`*

### a) Behauptung des Audits

Die drei v3-Provider der Sonde fangen den Prozessor als rohes `this`; läuft ein Client nach
Ablauf der 2-Sekunden-Stoppfrist abgelöst weiter, kann sein Provider einen bereits
zerstörten `SondeProcessor` berühren (`BEFUNDE.md:398`).

### b) Quellkette am HEAD

**Station 1 — die drei Lambdas, ohne Schleuse.**
`eq-copilot/plugin/sonde/SondeProcessor.cpp:139-142`, in der Initialisierungsliste:

```
      controlV3 ([this] { return v3Hello(); }, v3PipeName, {},
                 [this] { return v3Status(); }, {}, {}, brokerServerErwartung()),
      telemetryV3 ([this] { return v3TelemetryHello(); }, v3PipeName, {},
                   brokerServerErwartung()),
```

Zum Vergleich derselbe Ort bei Gen, `eq-copilot/plugin/src/PluginProcessor.cpp:188-193`:

```
      callbackSchleuse (std::make_shared<nakama::ipc::CallbackSchleuse>()),
      controlV3 ([this, s = callbackSchleuse]
                 {
                     if (auto zug = s->betreten())
                         return v3Hello();
                     return nakama::ipc::ControlHello {};
                 },
```

Die Sonde hat kein `callbackSchleuse`-Mitglied; eine Suche über `SondeProcessor.h` findet
keinen Treffer für `CallbackSchleuse`.

**Station 2 — die Provider starten im Produktzweig.** `SondeProcessor.cpp:191-200`:

```
#if ! defined (NAKAMA_PHASE_B_TEST_NO_PRODUCT_V3)
    // Probeeq ist reiner Connector: beide v3-Verbindungen, kein Lifecycle und
    // kein Spawn. ...
    controlV3.start();
    telemetryV3.start();
```

**Station 3 — was die Provider anfassen.** `v3Hello()` steht bei `:1108-1132` und nimmt
`zustandSchloss` (`:1112`); `v3Status()` bei `:1134-1210`, und dort steht die scharfe
Zeile — `SondeProcessor.cpp:1143`:

```
        s.stateRevision = transaktion->revision();
```

`v3TelemetryHello()` bei `:1212-1219` ruft `v3Hello()` (`:1215`) und `controlV3.kopplung`
(`:1217`).

**Station 4 — der Destruktor wartet auf nichts davon.**
`SondeProcessor.cpp:212-230`, vollständig:

```
SondeProcessor::~SondeProcessor()
{
    briefkasten.stoppe();
    for (auto* p : getParameters())
        p->removeListener (this);
    workerLaeuft.store (false);
    workerWarte.notify_all();
    if (worker.joinable())
        worker.join();
    telemetryV3.stop();
    controlV3.stop();
}
```

Gen schließt an derselben Stelle die Schleuse — `PluginProcessor.cpp:500`:
`callbackSchleuse->schliessen();`, mit dem Kommentar `:491-499` („die Schleuse schliesst
NACH den drei `stop()` und VOR der Zerstoerung der Mitglieder").

**Station 5 — `stop()` löst nach 2 s ab.** `eq-copilot/plugin/core/ipc/ControlClient.cpp:119-136`:

```
    // `B-CC-12`: auf einen laufenden Callback wird hoechstens `kStopFristMs`
    ...
                ++k->zustand.stopFristUeberschritten;
                k->zustand.status = Status::getrennt;
            }
            thread.detach();
```

`thread.detach()` steht bei `ControlClient.cpp:133` und, wortgleich,
`TelemetryClient.cpp:328`. Die Frist ist zweimal 2000 ms:
`eq-copilot/plugin/core/ipc/controlclient/Intern.h:59`
(`inline constexpr int kStopFristMs = 2000;`) und
`eq-copilot/plugin/core/ipc/TelemetryClient.cpp:30`
(`constexpr int kStopFristMs = 2000;`).

**Station 6 — die Zerstörungsreihenfolge macht den Zugriff deterministisch bösartig.**
C++ zerstört Mitglieder in umgekehrter Deklarationsreihenfolge.
`eq-copilot/plugin/sonde/SondeProcessor.h`:

```
:523     nakama::ipc::ControlClient controlV3;
:524     nakama::ipc::TelemetryClient telemetryV3;
...
:535     std::unique_ptr<nakama::dsp::DspKern>                    dspKern;
:536     std::unique_ptr<nakama::transaktion::DspKernAusfuehrung> dspAusfuehrung;
:537     std::unique_ptr<nakama::transaktion::Transaktionskern>   transaktion;
```

`transaktion` (`:537`) stirbt **vor** `controlV3` (`:523`). Kehrt der Destruktorrumpf
nach dem abgelösten `stop()` zurück, wird `transaktion` zerstört, während der abgelöste
Clientthread noch in `v3Status()` stehen kann — und die nächste Zeile, die er ausführt,
ist `SondeProcessor.cpp:1143`, `transaktion->revision()` auf dem freigegebenen Objekt.
`zustand` (`:440`) und `zustandSchloss` (`:442`) sind dagegen **vor** den Clients
deklariert und leben in diesem Fenster noch; der Zugriff ist also nicht diffus, sondern
trifft genau die drei Heap-Mitglieder `:535-537`.

### c) Zusage wörtlich

**Verletzt.** `docs/beweise/NAK-246.md:1008-1016`, Entscheid R-D2:

> „R-D2: Kein Produkt-Callback greift nach Beginn der Prozessorzerstörung auf
> Prozessorzustand zu. Die Callbacks der Clients fangen den Prozessor nicht mehr als rohes
> this, sondern laufen durch eine Schleuse mit Besitz: ein Callback, der nach dem Schließen
> der Schleuse beginnt, wird abgewiesen (zählbar); ein Callback, der beim Schließen bereits
> läuft, wird vom Destruktor zu Ende gewartet … Die 2000-ms-Stoppfrist des Clients
> (B-CC-12) und die Ablösung des Threads bleiben; die abgelöste Laufzeit hält dann nur noch
> Callbacks, die niemanden mehr berühren."

Der Satz ist ohne Einschränkung auf Gen formuliert. Der Kopfkommentar der Schleuse sagt
dasselbe in eigener Sprache, `eq-copilot/plugin/core/ipc/controlclient/Schleuse.h:6-11`:
„die Produkt-Lambdas darin fingen den Prozessor bis NAK-246 als rohes `this`. Ein Callback,
der die Frist ueberlebte, lief danach auf einem Prozessor weiter, den der Host laengst
zerstoert hatte (Auditbefund D2, Use-after-free)." Die Sonde-Hälfte ist genau dieser
Zustand, unrepariert.

### d) Heutige Abdeckung

- **B14 `EqCopSonde012ProjectReloadTest`** (`tools/beweise.ps1:745`) enthält den
  einschlägigen Fall M-07,
  `eq-copilot/plugin/tests/Sonde012ProjectReloadTest.cpp:456-475`. Sein Aufbauhelfer
  liefert ausdrücklich einen **Gen**-Prozessor, `:440`:
  `eqcop::EqCopilotProcessor* mainAnlegen (ProzessorPuffer& puffer, const std::string& pipe, const char* fall)`.
  Der Fall misst die Gen-Schleuse, nicht die Sonde.
- **Kein Bein baut heute eine Sonde mit laufenden v3-Clients.** `SondeProcessor.cpp:191`
  klammert `controlV3.start()`/`telemetryV3.start()` mit
  `#if ! defined (NAKAMA_PHASE_B_TEST_NO_PRODUCT_V3)`; **jedes** Konsolenziel setzt dieses
  Define (`eq-copilot/plugin/CMakeLists.txt:409`, `:439`, `:495`, `:511`, `:557`, `:592`,
  `:869`, `:1173`, `:1329`). Gen hat für genau diesen Zweck zwei Ausweichwege — einen
  Probe-Pipe-Testkonstruktor (`PluginProcessor.h:100`) und zwei Starthaken
  (`PluginProcessor.h:763` `v3StartFuerTest`, `:766` `v3TelemetrieStartFuerTest`). Die
  Sonde hat **keinen** davon; `grep FuerTest` über `SondeProcessor.h` findet als einzigen
  Starthaken `briefkastenStartenFuerTest` (`:361`), den Diagnosetakt.
- **Der Testzugang zu den Providern ist nur synchron.** `SondeProcessor.h:254-256`
  stellt `v3HelloFuerTest`/`v3StatusFuerTest` bereit — Aufrufe auf dem Testthread, nicht
  auf einem Clientthread; sie können die Lebensdauerfrage prinzipiell nicht stellen.
- **Phase 16 E** fuhr 36 Zyklen mit 72 Probeeq-Instanzen und misst Handles, Threads und
  PrivateBytes (`roh/phase-16-e-last-lebenslauf.md:67-69`) — **ohne** gehaltene Callbacks;
  die Arbeitsübersicht sagt das ausdrücklich (`BEFUNDE.md:812`: „begrenzter E-Abbau ohne
  gehaltene Callbacks ist **kein** Gegenbeweis"). Derselbe Bericht hält fest, dass Probeeq
  produktiv keinen Editor hat (`:61`, belegt an `SondeProcessor.h:158-159`).
- **Register:** NAK-246 führt die Gen-Hälfte als geschlossen; für die Sonde-Hälfte gibt es
  heute nur die Auditzeile.
- **Irreführender Kommentar:** keiner an dieser Stelle. `SondeProcessor.cpp:214-215`
  begründet korrekt nur die Briefkasten-Schleuse als ersten Destruktorschritt und erhebt
  keinen Anspruch auf die IPC-Provider.

### e) Urteil

**BESTÄTIGT**, mit zwei Präzisierungen.

1. **Die Auditzeile nennt den Zugriff allgemein; er ist enger und härter zugleich.** Wegen
   der Deklarationsreihenfolge (`SondeProcessor.h:523/:524` gegen `:535-537`) trifft er
   nicht irgendeinen Prozessorzustand, sondern deterministisch die drei
   `unique_ptr`-Mitglieder — und der einzige Produktpfad, der eines davon dereferenziert,
   ist `v3Status()` bei `SondeProcessor.cpp:1143`. Das ist gut für die Reparatur: der Fix
   braucht keine Umsortierung, nur die Schleuse.
2. **Die Erreichbarkeit im Beweissystem ist heute null.** Nicht nur fehlt der Test — es
   fehlt die Bauvoraussetzung, ihn zu schreiben (kein Testkonstruktor, kein Starthaken).
   Das gehört vor die Reparatur, nicht dahinter.

Ein späterer Commit hat den Befund nicht berührt: der einzige Diff seit `aff2d818` in
dieser Datei betrifft `prepareToPlay` und `reset()`.

### f) Kategorievorschlag

**DEFEKT** — R-D2 (`NAK-246.md:1008`) ist eine datierte, unbeschränkt formulierte
Entscheidzeile, und der Bau erfüllt sie für eine der beiden Prozessorfamilien nicht.

**Schwere high.** Ich stufe gegenüber dem Audit (critical) um eine Stufe herab, mit Grund:
der Zugriff setzt voraus, dass ein Provider die 2-Sekunden-Frist reißt — ein Zustand, den
der Client selbst als Ausnahme zählt (`stopFristUeberschritten`, `ControlClient.cpp:130`).
Kein Absturz ist beobachtet; Phase 04 sagt das selbst (`roh/phase-04-lebenslauf.md:44`:
„Es wird kein beobachteter Absturz behauptet"). Tritt die Bedingung ein, ist die Folge
allerdings ein echter Use-after-free — deshalb nicht tiefer als high, und in der
Reihenfolge trotzdem zuerst.

### g) Kleinster Fix

Denselben Mechanismus anwenden, den Gen schon trägt, ohne ihn zu erweitern.

1. `SondeProcessor.h` bekommt ein `std::shared_ptr<nakama::ipc::CallbackSchleuse>
   callbackSchleuse`, **vor** `controlV3` (`:523`) deklariert, damit es die Clients
   überlebt.
2. Die drei Lambdas in `SondeProcessor.cpp:139-142` fangen die Schleuse als `shared_ptr`
   mit und betreten sie vor dem ersten Besitzerzugriff — Form wörtlich wie
   `PluginProcessor.cpp:189-196`. Ein abgewiesener Zug liefert das leere Default-Objekt
   zurück, wie bei Gen.
3. `~SondeProcessor()` schließt sie **nach** `telemetryV3.stop()`/`controlV3.stop()`
   (`:228-229`) und vor dem Ende des Rumpfes, also vor jeder Mitgliederzerstörung — die
   Stelle, die `PluginProcessor.cpp:500` bei Gen einnimmt.

**Mitzuprüfende Beziehungen:** starten↔stoppen (`:195-196` gegen `:228-229`) und
verbinden↔trennen — `ladestart` ruft `controlV3.reconnect()`/`telemetryV3.reconnect()`
(`SondeProcessor.cpp:1104-1105`); ein Reconnect darf eine noch offene Schleuse nicht
schließen und eine geschlossene nicht wieder öffnen.

**Voraussetzung, die zum selben Änderungssatz gehört:** die Sonde braucht die zwei
Testzugänge, die Gen hat — einen Konstruktor auf einen Probe-Pipenamen
(Muster `PluginProcessor.h:100`) und Starthaken (Muster `PluginProcessor.h:763`, `:766`).
Ohne sie ist der Fix unbeweisbar. Der Probe-Namensraum ist dabei Pflicht (CLAUDE.md,
„Probe-Pipe ist nie Produktions-Pipe"); `PluginProcessor.h:95-98` beschreibt die
Fail-closed-Regel, die zu übernehmen ist.

### h) Ort des Rotbeweises

**Wie er ohne Speicherinstrumentierung deterministisch rot wird.** Genau so, wie NAK-246
es für Gen schon gebaut hat (`Sonde012ProjectReloadTest.cpp:456-475`) und wie R-D2 es
vorschreibt (`NAK-246.md:1013-1015`: „Produkt-Callback vor dem Besitzerzugriff festhalten,
Prozessor über die Frist abbauen, Callback freigeben … mit Wachobjekt/Kanarienvogel
messbar, kein ASan nötig"):

1. Sonde gegen einen Testserver im Probe-Namensraum bauen, v3-Clients über den neuen
   Starthaken starten.
2. Einen Haken im Provider **vor** dem ersten Besitzerzugriff auf eine Schranke stellen —
   bei `v3Status()` vor `SondeProcessor.cpp:1143`; die Schranke gehört dem Bein und
   überlebt den Prozessor, wie `Schranke gate` in
   `Sonde012ProjectReloadTest.cpp:469`.
3. Den Prozessor aus dem Testthread zerstören. `stop()` reißt die 2000-ms-Frist
   (`Intern.h:59`), löst ab (`ControlClient.cpp:133`) und kehrt zurück; die Mitglieder
   sterben.
4. Kanarienvogel statt Speicherinstrumentierung: das Ziel des Zugriffs ist ein
   `unique_ptr`-Mitglied, und `~unique_ptr` **setzt seinen Zeiger nicht** auf null. Der
   Beweis braucht deshalb eine eigene Marke: ein Testkern, dessen Destruktor ein
   `std::atomic<bool> lebt` auf false setzt, und eine Zählung im Provider, ob er nach
   `lebt == false` noch gelaufen ist. Das ist deterministisch und ohne ASan messbar.
5. Schranke freigeben, Zähler lesen. **Heute rot:** der Provider läuft nach Eigentümerende.
   Nach dem Fix ist er abgewiesen (Zähler `abgewiesen` der Schleuse steigt) oder der
   Destruktor hat ihn zu Ende gewartet (`gewartetMs` > 0) — beides zählbar, wie
   `Schleuse.h:17-22` es beschreibt.

**Kann ein Bein ihn heute erreichen? Nein.** Weder A16 `EqCopProbeeqNullTest`
(`tools/beweise.ps1:688`) noch B7 `EqCopTransactionTest` (`:728`) noch B14 (`:745`) kann
eine Sonde mit laufenden v3-Clients bauen — `NAKAMA_PHASE_B_TEST_NO_PRODUCT_V3` steht auf
allen Konsolenzielen, und die Sonde hat weder Testkonstruktor noch Starthaken. Der
Rotbeweis ist deshalb zweistufig: erst die Testzugänge, dann der Fall. Sinnvoller Ort ist
ein eigenes Sonde-Lebenslaufbein in der B14-Familie oder eine Erweiterung von B10
`EqCopIpcTest` (`:738`), dessen Behauptungszeile heute den C++-v3-Vertrag trägt; die neue
Behauptung gehört dorthin, wo der Fall gefahren wird.

---

## 2. T3-04-03 — das Messpunktpanel kann Editor und Prozessor überleben (W09)

*Befundzeile: `BEFUNDE.md:400` · Skeptiker Phase 10: `BEFUNDE.md:525` · Herleitung:
`roh/phase-04-lebenslauf.md:60-70` · Callbackinventar:
`roh/phase-04-lebenslauf.md:132` · Arbeitsübersicht: `BEFUNDE.md:812`*

### a) Behauptung des Audits

Das asynchron gehaltene `MesspunktPanel` kann den Editor und danach den Prozessor
überleben; sein Destruktor ruft ungeprüft `proz.setzeBindung` auf, und der SafePointer
schützt erst das nachfolgende Neuzeichnen (`BEFUNDE.md:400`).

### b) Quellkette am HEAD

**Station 1 — der Panel-Typ hält eine rohe Prozessorreferenz.**
`eq-copilot/plugin/src/PluginEditor.cpp:625-627`:

```
    struct MesspunktPanel : juce::Component, juce::ComboBox::Listener, juce::TextEditor::Listener
    {
        EqCopilotProcessor& proz;
```

**Station 2 — sein Destruktor schreibt.** `PluginEditor.cpp:688`:

```
        ~MesspunktPanel() override { uebernehmen(); }
```

und `uebernehmen()` bei `:700-709`, mit dem Zugriff bei `:707`:

```
            if (proz.setzeBindung (rolle, label, paarId) && geaendert)
```

Die linke Seite des `&&` läuft **immer**; `setzeBindung` nimmt sofort einen Mutex —
`eq-copilot/plugin/src/prozessor/State.cpp:304`: `std::lock_guard<std::mutex> l (bindungMutex);`.
Auch ein unveränderter Eingang kommt dort an: der Frühausstieg wegen fehlender Änderung
liegt **innerhalb** der Sperre, `State.cpp:318-319`.

**Station 3 — der SafePointer deckt etwas anderes ab.** `PluginEditor.cpp:734-742`:

```
    juce::CallOutBox::launchAsynchronously (
        std::make_unique<MesspunktPanel> (
            processor, s,
            [safe = juce::Component::SafePointer<EqCopilotEditor> (this)]
            {
                if (auto* editor = safe.getComponent())
                    editor->uiDirty = true;
            }),
        messpunktKnopf.getBounds(), this);
```

Der `SafePointer` sitzt im **Änderungs-Callback** (`geaendert`), also in der *rechten*
Seite des `&&` bei `:707`. Der erste Konstruktorparameter ist `processor` — eine rohe
Referenz.

**Station 4 — der Kommentar darüber behauptet mehr, als der Code leistet.**
`PluginEditor.cpp:731-733`, wörtlich:

```
    // SafePointer statt rohem this: der Panel-Destruktor ruft uebernehmen()
    // auch dann, wenn FL das Editorfenster MIT offenem Popover schließt —
    // der Editor ist dann schon im Abbau (Component nullt SafePointer sofort).
```

Der Satz stimmt für den Editor und ist für den **Prozessor** falsch: `proz` (`:627`) ist
kein SafePointer und wird bei `:707` vor dem geschützten Teil dereferenziert. Das ist der
irreführende Kommentar dieses Befundes, `eq-copilot/plugin/src/PluginEditor.cpp:731-733`.

**Station 5 — JUCE besitzt das Panel und zerstört es verzögert.** Lokaler JUCE-Stand im
Baum. `eq-copilot/build/_deps/juce-src/modules/juce_gui_basics/windows/juce_CallOutBox.cpp:62-87`:
`class CallOutBoxCallback final : public ModalComponentManager::Callback` mit
`std::unique_ptr<Component> content;` bei `:83` — das Panel gehört diesem Callback, nicht
dem Editor. Beim Editorabbau meldet JUCE nur ab:
`eq-copilot/build/_deps/juce-src/modules/juce_gui_basics/components/juce_ModalComponentManager.cpp:70-78`
(`componentBeingDeleted` → `cancel()`) und `:81-90`:

```
    void cancel()
    {
        if (isActive)
        {
            isActive = false;

            if (auto* mcm = ModalComponentManager::getInstanceWithoutCreating())
                mcm->triggerAsyncUpdate();
        }
    }
```

Zerstört wird erst in `handleAsyncUpdate`, `juce_ModalComponentManager.cpp:196-212`; der
`std::unique_ptr<ModalItem> deleter` bei `:204` nimmt den Eintrag aus dem Stapel, und mit
seinem Ende sterben `callbacks` → `CallOutBoxCallback` → `content` → `~MesspunktPanel`.

**Station 6 — der VST3-Wrapper räumt die CallOutBox nicht synchron.**
`eq-copilot/build/_deps/juce-src/modules/juce_audio_plugin_client/juce_audio_plugin_client_VST3.cpp:2326-2327`:

```
                    PopupMenu::dismissAllActiveMenus();
                    pluginEditor->processor.editorBeingDeleted (pluginEditor.get());
```

`dismissAllActiveMenus` betrifft `PopupMenu`, nicht `CallOutBox`. Zerstört der Host in
derselben Message-Thread-Runde danach den Prozessor, läuft `~MesspunktPanel` erst in der
Folgerunde — auf einem toten `proz`.

**Station 7 — wo das Panel überhaupt lebt.** Der Einstieg ist `messpunktKnopf`; der Knopf
steht in der Legacy-Liste und ist nur dort sichtbar, `PluginEditor.cpp:840-846`. Probeeq
hat keinen Editor: `eq-copilot/plugin/sonde/SondeProcessor.h:158-159`
(`createEditor() override { return nullptr; }`, `hasEditor() const override { return false; }`).
Der Befund betrifft also ausschließlich eine **Gen**-Instanz in der Legacy-Rolle.

### c) Zusage wörtlich

**Die Quellen schweigen.** Gesucht und nicht gefunden: eine Lebensdauerregel für ein
ausgelagertes modales Panel in `docs/FL-Nakama-Sonden-Design-Entwurf.md` (Suche nach
„Detach", „Stoppfrist", „Besitz", „Lebensdauer" liefert nur `:1839` — eine Feldtabelle —
und `:3868` zum Broker-Coordinator), in `design/docs/funktions-und-bedien-blueprint.md`
und in `docs/plugin-wissen.md`.

R-D2 (`NAK-246.md:1008`) regelt ausdrücklich **Client-Callbacks**; die Skeptikerprüfung
hat zu Recht abgelehnt, ihn stillschweigend auf jeden JUCE-Dialog auszudehnen
(`roh/phase-04-lebenslauf.md:70`). CLAUDE.md „State bleibt verlustfrei" (`:201-203`)
betrifft unbekannte Major-Versionen und Dirty-Meldung, nicht die Lebensdauer eines
Dialogs. **Keine Zusage ist gebrochen.**

### d) Heutige Abdeckung

- **Kein Bein fährt einen modalen Abbau.** Weder B15 `EqCopShot` (`tools/beweise.ps1:746`,
  Behauptung: gerenderte Sichtsätze) noch B14 (`:745`) noch B2 (`:693`) baut eine
  CallOutBox auf und zerstört danach Editor und Prozessor. Phase 04 hat in den
  durchsuchten Plugintests keinen gefunden (`roh/phase-04-lebenslauf.md:70`).
- **B15 rendert ohne Popover.** `eq-copilot/plugin/tests/ShotTestMain.cpp` fährt Sichten
  über gesetzte Fixtures (`:114-118`); ein `launchAsynchronously` kommt darin nicht vor.
- **Phase 16 E** erzeugte und zerstörte 108 Gen-Editorobjekte
  (`roh/phase-16-e-last-lebenslauf.md:67`) — ohne offenes Popover und ohne sichtbares
  Hostfenster (`:61`). Das ist kein Gegenbeweis.
- **Register:** keine Zeile; T3-02-04 sitzt am selben Destruktor, betrifft aber die
  Kürzung gespeicherter Strings und ist davon unabhängig
  (`roh/phase-04-lebenslauf.md:70`).
- **Irreführender Kommentar:** `PluginEditor.cpp:731-733` (siehe b, Station 4).

### e) Urteil

**BESTÄTIGT**, präzisiert in zwei Punkten.

1. **Der Kommentar am Übergabepunkt ist der eigentliche Grund, warum der Fall übersehen
   wurde.** `PluginEditor.cpp:731-733` liest sich, als sei der Destruktorpfad abgesichert;
   abgesichert ist nur das Neuzeichnen. Wer den Fix baut, muss diesen Kommentar mit
   ändern, sonst steht dort nach der Reparatur eine halbe Wahrheit zur anderen Hälfte.
2. **Der Befund ist auf Gen-Legacy eingegrenzt.** Probeeq hat keinen Editor
   (`SondeProcessor.h:158-159`), und der Knopf ist nur außerhalb der Main-Fläche sichtbar
   (`PluginEditor.cpp:840-846`). Das verkleinert den Prüfumfang erheblich.

Die zulässige Hostreihenfolge, die der Befund braucht — Editor zerstören, Prozessor
zerstören, dann die modale Aufräumnachricht verarbeiten —, ist erreichbar, aber **nicht
garantiert**: läuft dazwischen eine Runde der Nachrichtenschleife, stirbt das Panel vorher
und alles ist in Ordnung. Das ist kein Gegenargument, sondern die Beschreibung der
Bedingung.

### f) Kategorievorschlag

**LÜCKE** — keine gelesene Zusage regelt die Lebensdauer dieses Panels; es fehlt die Regel,
nicht die Erfüllung.

**Schwere high.** Herabstufung gegenüber dem Audit (critical) mit demselben Maßstab wie
oben: die Folge ist ein Zugriff auf freigegebenen Speicher samt Sperrnahme auf einem toten
Mutex, also ein realistischer Hostabsturz — aber die auslösende Reihenfolge ist eine von
mehreren möglichen, und niemand hat sie gemessen. Sie gehört mit T3-04-01 in dieselbe
erste Etappe.

### g) Kleinster Fix

Den Prozessorzugriff an einen geprüften lebenden Besitzer binden, ohne die Oberfläche zu
verändern.

1. `MesspunktPanel` hält statt `EqCopilotProcessor& proz` (`PluginEditor.cpp:627`) einen
   `juce::Component::SafePointer<EqCopilotEditor>` — denselben, den der Änderungs-Callback
   schon führt (`:737`). `uebernehmen()` (`:700-709`) holt den Prozessor über den Editor
   und kehrt ohne jeden Zugriff zurück, wenn der Editor weg ist. Das ist eine Zeile
   Mechanik und kein neues Muster: der Editor lebt genau so lange wie sein Prozessor
   mindestens — er wird laut JUCE-Vertrag vorher zerstört.
2. Der Kommentar `:731-733` wird auf das berichtigt, was dann gilt.
3. `PluginEditor.cpp:734` übergibt entsprechend nicht mehr `processor`, sondern den
   SafePointer.

**Mitzuprüfende Beziehungen:** öffnen↔schließen (der Knopf öffnet das Panel bei `:734`,
der Destruktor `:688` schließt es) und speichern↔laden — `setzeBindung` schreibt
persistenten Zustand und meldet Dirty (`State.cpp:358-362`); ein Abbau **ohne** Änderung
darf weiterhin nichts melden (`State.cpp:318-319` hält das heute schon, und der Fix darf
das nicht verschieben).

### h) Ort des Rotbeweises

**Bein B15 `EqCopShot`** (`tools/beweise.ps1:746`) ist der einzige Ort im Kanon, an dem
ein echter Editor gebaut wird; alternativ ein neuer Fall in der B14-Familie (`:745`), wo
schon Prozessorabbau gefahren wird. Der Fall:

1. Gen in Legacy-Rolle, Editor öffnen, `zeigeMesspunkt()` auslösen — die CallOutBox steht
   (`PluginEditor.cpp:734`).
2. Im Panel eine echte Änderung setzen (andere Rolle oder anderes Label), damit
   `uebernehmen()` bei `:707` nicht schon an `State.cpp:318` abbiegt.
3. Editor zerstören, danach Prozessor zerstören — **ohne** die Nachrichtenschleife dazwischen
   zu fahren.
4. Erst danach `juce::MessageManager` einen Durchlauf geben, damit
   `handleAsyncUpdate` (`juce_ModalComponentManager.cpp:196`) das Panel zerstört.

**Die Mutation, die die Zusage bricht**, ist die Zeile `PluginEditor.cpp:688`
(`~MesspunktPanel() override { uebernehmen(); }`) beziehungsweise nach dem Fix die
Lebendprüfung darin: entfernt man sie, muss der Fall rot werden. Der Rotbeweis fällt
**nicht** an einem Nebeneffekt wie `uiDirty` — der ist schon heute geschützt (`:739`) —,
sondern am Prozessorzugriff selbst; als Marke dient derselbe Kanarienvogel wie in
Abschnitt 1 h): ein `std::atomic<bool>` im Prozessor, das sein Destruktor löscht, und eine
Zählung im Panel-Destruktor. **Heute rot.** Gegenfall im selben Bein: Panel schließen,
*dann* Editor, *dann* Prozessor — muss grün bleiben und darf kein Dirty melden.

---

## 3. T3-02-06 — MainProject-Bestände beim Rollenwechsel (W22)

*Befundzeile: `BEFUNDE.md:367` · Skeptiker Phase 10: `BEFUNDE.md:510` · Herleitung:
`roh/phase-02-zustand.md:102-115` · Produktfrage: `roh/phase-02-zustand.md:425` (P-01),
`roh/phase-10-skeptiker.md:129` (P10-02) · Kartenentwurf PF11-03: `BEFUNDE.md:225-230` ·
Arbeitsübersicht: `BEFUNDE.md:825`*

### a) Behauptung des Audits

Beim Wechsel main → legacy bleiben sechs MainProject-Bestände im Speicher, verschwinden
aber aus dem Save; die Rückkehr nach main stellt sie nur ohne zwischenzeitlichen Reload
wieder her (`BEFUNDE.md:367`). Die Arbeitsübersicht fasst das kürzer:
„Main→Legacy→Main behält live Daten, verliert sie nach Save/Reload" (`BEFUNDE.md:825`).

### b) Quellkette am HEAD

**Station 1 — der Rollenwechsel löscht genau eine Sammlung, und zwar sofort.**
`eq-copilot/plugin/src/prozessor/State.cpp:320-323`, in `setzeBindung` (Funktionskopf `:287`):

```
        zustand.common = neu;
        if (klasse != nakama::state::Klasse::main)
            zustand.mainProjectMitglieder.clear();
        mainMitglieder = zustand.mainProjectMitglieder;
```

Die geleerte Kopie geht unmittelbar ins Modell, `State.cpp:358-359`:

```
    werteSourcesPublikationAus (
        sourcesModel.setzePersistenteMitglieder (mainMitglieder, generation, folge));
```

**Station 2 — die sechs anderen Bestände bleiben stehen.** `eq-copilot/plugin/state/NakamaState.h`
führt sie im selben Zustand: `manuellePassagen` (`:451`), `sourceIntents` (`:456`),
`schutzangaben` (`:457`), `intentBeziehungen` (`:458`), `intentBestandRevision` (`:465`),
`assistent` (`:471`). `setzeBindung` fasst keines davon an — der zitierte Block `:320-323`
ist die einzige Zustandsänderung an MainProject-Daten in der Funktion.

**Station 3 — der Serializer wirft das ganze Kind weg.**
`eq-copilot/plugin/state/NakamaState.cpp:827-828` prüft die Klasse, und `:1009-1012`:

```
    else if (mainProject.isValid())
    {
        kopie.removeChild (mainProject, nullptr);
    }
```

**Station 4 — der Leser füllt die sechs nur für Main.** Die Lokalen sind leer
vorinitialisiert, `NakamaState.cpp:1646-1649`:

```
    std::vector<Schutzangabe> mainSchutz;
    std::vector<IntentBeziehung> mainKanten;
    juce::int64 mainIntentRevision = 0;
    Assistentenzustand mainAssistent {};
```

Gefüllt werden sie ausschließlich innerhalb der drei Wachen `if (istMain)` bei
`NakamaState.cpp:1536`, `:1583` und `:1650`; `istMain` entsteht bei `:1520`
(`const bool istMain = c.klasse == Klasse::main;`). Übernommen wird bei `:2017-2023`:

```
    aus.mainProjectMitglieder = std::move (mainMitglieder);
    aus.manuellePassagen      = std::move (mainPassagen);
    aus.sourceIntents         = std::move (mainIntents);
    aus.schutzangaben         = std::move (mainSchutz);
    aus.intentBeziehungen     = std::move (mainKanten);
    aus.intentBestandRevision = mainIntentRevision;
    aus.assistent = mainAssistent;
```

Ein als `legacy` gespeicherter Stand hat kein MainProject-Kind (Station 3) und wird als
`legacy` gelesen — alle sieben Werte kommen leer zurück, auch nach dem Rückwechsel nach
`hub`.

**Station 5 — die Grenze, die das Löschen im Save rechtfertigt.** Die Kind-Matrix des
Vertrags, `eq-copilot/schemas/state/nakama-state-v2.md:95-101`, Zeile `:100`:
`| legacy | Pflicht | verboten | verboten | verboten | verboten |`. Der Leser erzwingt sie,
`NakamaState.cpp:1523`:
`if (! istMain && nMain > 0)  { grund = juce::String ("MainProject is not allowed for ") + wort (c.klasse); return false; }`.
Ein Legacy-Save **mit** MainProject wäre also nicht nur unschön, sondern read-only beim
nächsten Laden. Das Weglassen ist vertragsrichtig; offen ist nur, was mit den Daten
geschieht.

### c) Zusage wörtlich

**Die Quellen schweigen zur Lebensdauer.** Der Vertrag regelt den Ort
(`nakama-state-v2.md:100`) und den Inhalt der Felder (`:106-112` für
`confirmed_members_v1`, `:114-120` für `manual_passages_v1`), nicht den Fall
„bewusstes Verlassen und Wiederbetreten der Main-Rolle".

**CLAUDE.md „State bleibt verlustfrei"** (`:201-203`) ist **nicht** verletzt, wörtlich:

> „Unbekannte Major-Versionen oder unzulässige Matrizen werden read-only mit Originalbytes
> gehalten; jede persistente Änderung meldet dem Host Dirty-State."

Beide Hälften halten: der Stand ist kein fremder Major, und der Rollenwechsel meldet Dirty
(`State.cpp:358-362` über `werteSourcesPublikationAus`, `Ipc.cpp:1489` `meldeHostDirty()`).
Der Entwurf und der Blueprint enthalten zu dieser Bedienfolge nichts.

### d) Heutige Abdeckung

- **Bein B2 `EqCopStateMigrationTest`** (`tools/beweise.ps1:693`) hat die einzige
  Rollenwechselprüfung: `eq-copilot/plugin/tests/StateMigrationTestMain.cpp:2331-2334`,
  Folge `pre → post → hub` mit leerem MainProject; gemessen wird die Dirty-Zahl, nicht der
  Bestand. Eine Folge `hub → sensor → hub` kommt in der Datei nicht vor (Suche über alle
  `setzeBindung`-Aufrufe: `:2043`, `:2331`, `:2332`, `:2334`, `:2348`, `:2349`, `:2358`,
  `:2430`).
- **Bein B14** (`:745`) prüft laut Behauptungszeile Save/Load der bestätigten
  Quellidentität und der User-Label — aber innerhalb der Main-Rolle, nicht über einen
  Rollenwechsel hinweg.
- **Phase 16 C** hat den Rollen-/Intentweg über die gebaute API gefahren
  (`roh/phase-16-c-persistenz.md:46`) und dabei Revisionen nach Reload gemessen; der
  Klassenwechsel hub → sensor → hub war nicht Teil davon.
- **Register:** keine eigene Zeile; NAK-312 führt den Punkt gesammelt
  (`docs/offene-punkte.md:316`).
- **Irreführender Kommentar:** keiner. `State.cpp:332-343` begründet die
  Klassifikationsrichtung in beide Richtungen ehrlich und schweigt zum Datenbestand — das
  ist genau die Lücke, keine Falschaussage.

### e) Urteil

**PRÄZISIERT.** Die Zeile `BEFUNDE.md:367` ist in ihrer Langfassung korrekt und nennt die
Ausnahme selbst („Nur confirmed_members_v1 wird bereits beim Rollenwechsel gelöscht").
Die Kurzfassung der Arbeitsübersicht (`BEFUNDE.md:825`) und der Text der Karte U49 sind
es **nicht**:

> Der Live-Rückweg erhält die Daten nicht vollständig. `State.cpp:322` löscht
> `mainProjectMitglieder` — die **bestätigten Quellen** — sofort und endgültig, gleich ob
> gespeichert wird oder nicht. Was den Live-Rückweg übersteht, sind die anderen sechs
> Sammlungen (`NakamaState.h:451`, `:456`, `:457`, `:458`, `:465`, `:471`).

Für einen Musiker ist das genau der Punkt, den er zuerst bemerkt: die Liste seiner
angemeldeten Messpunkte. U49 verspricht heute, live bleibe alles erhalten; das ist für den
sichtbarsten Bestand falsch. Der Rest der Behauptung trägt.

### f) Kategorievorschlag

**LÜCKE** — keine Zusage verletzt, es fehlt eine Regel für eine Bedienfolge, die es gibt.
Die Skeptikerprüfung ist zu demselben Ergebnis gekommen (`BEFUNDE.md:510`).

**Schwere medium.** Verlorener Projektinhalt ohne Warnung, erreichbar über zwei gewöhnliche
Handgriffe (Rollenwahl im Messpunkt-Panel, Speichern) — aber kein Audiofehler, kein
fremder State beschädigt, und der Verlust ist durch erneutes Anmelden heilbar.

### g) Kleinster Fix

Der Fix hängt an der Antwort auf U49; beide Richtungen sind klein und liegen an derselben
Stelle.

**Bei „erhalten" (Weg 1, Default der Karte und des Kartenentwurfs PF11-03,
`BEFUNDE.md:230`):** Die MainProject-Daten überleben die Legacy-Zeit im Stand, werden aber
nicht als MainProject-Kind geschrieben — das verböte die Kind-Matrix
(`nakama-state-v2.md:100`). Der Ort ist deshalb der Schreiber: `NakamaState.cpp:1009-1012`
verlegt die sieben Sammlungen in ein **eigenes, versioniertes Kind** für den ruhenden
Bestand statt sie zu verwerfen, und der Leser stellt sie beim Rückwechsel nach `main`
wieder her. Das ist eine Vertragsänderung: neues Kind heißt nach `nakama-state-v2.md:103`
Root-Versionierung, also Schema, Leser, Schreiber, Fixtures und Migration in einem Satz.
Gleichzeitig entfällt `State.cpp:321-322` als Löschstelle; stattdessen wandert der Bestand
dorthin. **Das ist nicht der kleinste Weg.**

**Der kleinste Weg, der die Antwort „erhalten" trotzdem erfüllt,** ist: den Bestand **im
Prozessor** über die Legacy-Zeit halten (also `State.cpp:321-322` streichen und die sieben
Sammlungen unangetastet lassen) und beim Speichern als Legacy ausdrücklich nicht
schreiben. Dann gilt „erhalten" für die laufende Instanz vollständig und symmetrisch —
auch für `confirmed_members_v1` — und ein Save/Load im Legacy-Zustand verliert sie
weiterhin. Das ist ehrlich, solange die Oberfläche es sagen kann; heute kann sie es nicht,
und die Fläche ist geparkt. Deshalb halte ich diesen Weg für unvollständig und die
Vertragsvariante für die richtige, sobald U49 auf „erhalten" steht.

**Bei „bewusst löschen" (Weg 2):** `State.cpp:321-322` wird um die sechs übrigen
Sammlungen erweitert — sechs Zeilen im selben Block, unter derselben Sperre. Save und Load
brauchen keine Änderung, weil der Schreiber ohnehin schon alles verwirft
(`NakamaState.cpp:1011`). Dann sind Live-Rückweg und Reload-Rückweg identisch leer, und
die Asymmetrie ist weg.

**Mitzuprüfende Beziehungen in beiden Fällen:** speichern↔laden (Schreiber
`NakamaState.cpp:827-1012` gegen Leser `:1520-2023`) und aktivieren↔abklingen — der
Rollenwechsel publiziert ins Modell (`State.cpp:358-359`); die Publikation muss zur
gewählten Regel passen, sonst zeigt die Fläche einen anderen Bestand als der Stand.
Zusätzlich: **fremde Bytes bleiben bytegleich.** Unbekannte Eigenschaften in einem
bekannten Kind sind additiv und werden erhalten (`nakama-state-v2.md:103`); keiner der
beiden Wege darf daran rühren.

### h) Ort des Rotbeweises

**Bein B2 `EqCopStateMigrationTest`** (`tools/beweise.ps1:693`), neuer Fall neben
`StateMigrationTestMain.cpp:2331-2334`:

1. Main mit gefüllten Beständen aufbauen: mindestens eine bestätigte Quelle, eine manuelle
   Passage, ein Source-Intent, eine Schutzangabe, eine Intent-Beziehung, eine
   Bestandsrevision > 0 und ein offener Assistentenschritt — alle sieben, nicht nur die
   sichtbaren.
2. `setzeBindung ("sensor", …)` — Live-Bestand aller sieben messen.
3. `setzeBindung ("hub", …)` — Live-Bestand erneut messen.
4. Getrennter Durchgang: nach Schritt 2 speichern, in eine frische Instanz laden,
   `setzeBindung ("hub", …)`, Bestand messen.

**Erfolgskriterium:** die gewählte Regel — beide Wege ergeben denselben Bestand.
**Heute rot in beiden Richtungen:** Weg 3 liefert sechs volle Sammlungen plus null
Mitglieder, Weg 4 liefert siebenmal leer.

**Die Mutation, die die Zusage bricht**, ist `eq-copilot/plugin/src/prozessor/State.cpp:322`
(`zustand.mainProjectMitglieder.clear();`) beziehungsweise die Zeilen, die nach dem Fix
dort stehen — nicht der Serializer, denn der tut mit `NakamaState.cpp:1011` genau das
vertraglich Richtige. Gegenwache: Bein **B14** (`:745`), dessen Behauptungszeile heute
Save/Load der bestätigten Quellidentität trägt und um den Rollenwechsel zu ergänzen ist.
Dazu ein Gegenfall, der die Verlustfreiheit nicht ausweitet: ein Stand mit einer
unbekannten additiven Eigenschaft in `Common` muss durch die ganze Folge bytegleich
bleiben.

---

## 4. T3-07-02 — Legacy zeigt Solo/Puls, Main führt es aus, der Aus-Weg verschwindet (W22)

*Befundzeile: `BEFUNDE.md:437` · Skeptiker Phase 10: `BEFUNDE.md:534` · Herleitung:
`roh/phase-07-modell.md:32`, Handgriffkarte `:213`, `:219` · Registerzeile NAK-179:
`docs/offene-punkte.md:205` · Arbeitsübersicht: `BEFUNDE.md:825`*

### a) Behauptung des Audits

Die sichtbaren Solo/Puls-Aktionen der Legacy-Fläche sind durch die Main-Klassifikation
gesperrt, melden aber Engagement; ein dort vorgemerkter Marker wird beim Wechsel zu `hub`
ausführbar, während Aus-Knopf und Timer-Rücknahmen verschwinden (`BEFUNDE.md:437`).

### b) Quellkette am HEAD

**Station 1 — der sichtbare Einstieg liegt in der Legacy-Fläche.**
`eq-copilot/plugin/src/PluginEditor.cpp:508-519` baut die Befundliste mit dem
Markierungs-Callback; sie hängt am `hinweisKnopf`, und der steht in der Legacy-Liste bei
`:840-843`.

**Station 2 — die Aktion prüft die Klassifikation nicht.**
`PluginEditor.cpp:754-815` (`schalteMarkierung`) enthält keine Abfrage von
`holeKlassifikation()` oder `istMainFlaeche()`. Sie reicht direkt ein, `:800`:

```
    processor.markierungEinreichen (auftrag);
```

und meldet dann, `:807-812`:

```
    statusMeldung = (modus == MarkierungsModus::puls
                         ? u8 ("Einfärben: PULS — der Bereich schwillt im Takt an.")
                         : u8 ("Einfärben: SOLO — nur dieser Bereich spielt."))
                    + deckelNotiz
                    + (processor.markierungEchtzeitOk() ? juce::String()
                                                        : u8 (" Startet mit der Wiedergabe."));
```

Der einzige Vorbehalt in diesem Text kommt aus `markierungEchtzeitOk()` —
`eq-copilot/plugin/src/PluginProcessor.h:1081`:
`bool markierungEchtzeitOk() const { return echtzeitOk.load(); }`. Das ist der
Echtzeitbeweis, **nicht** die Klassifikation. In Legacy ist der Satz „nur dieser Bereich
spielt" deshalb unbedingt falsch: nichts spielt anders.

**Station 3 — das Audio verlangt Main.** `markierungEinreichen` leitet weiter,
`PluginProcessor.h:1077`:
`void markierungEinreichen (const MarkierungsAuftrag& a) { markierung.reicheEin (a); }`.
Gefärbt wird nur bei erlaubter Lage, `eq-copilot/plugin/src/PluginProcessor.cpp:874-881`:

```
    const bool erlaubt = istMainKlassifiziert.load (std::memory_order_relaxed)
                      && (echtzeitOk.load (std::memory_order_relaxed)
                          || testEchtzeit.load (std::memory_order_relaxed))
                      && spielt
                      && aufnahmeAus
                      && ! isNonRealtime()
                      && (editorOffen.load (std::memory_order_relaxed)
                          || testEchtzeit.load (std::memory_order_relaxed));
```

`istMainKlassifiziert` spiegelt den Automaten, `eq-copilot/plugin/src/prozessor/State.cpp:272`:
`istMainKlassifiziert.store (lebenslauf.audioAusnahmeErlaubt());`, gesetzt in
`spiegleKlassifikation()`, die `setzeBindung` bei `State.cpp:344` aufruft.

**Station 4 — der Wunsch bleibt scharf, bis jemand ihn zurücknimmt.**
`eq-copilot/plugin/src/HoerMarkierung.h:472-491` übernimmt den Auftrag in `lokal`; das
Ziel entsteht bei `:493-499`:

```
        const bool zielAn = ! ausGewuenscht
                         && gelesenNr != 0
                         && ! oversizeRiegel        // E-01: Ziel 0 und kein Wiedereinblenden
                         && erlaubt                 // NAK-47: Erlaubnisverlust = Ziel 0
                         && lokal.modus != MarkierungsModus::aus
                         && lokal.sektionen > 0
                         && lokal.fs == fsAktuell;
```

`erlaubt` ist **ein Faktor unter sieben**; wird er später wahr, wird `zielAn` wahr, ohne
dass der User etwas tut. `ausGewuenscht` wird nur durch einen ausdrücklichen Aus-Auftrag
gesetzt (`:485`), und die Samplerate ändert sich beim Rollenwechsel nicht. Aufgerufen wird
`verarbeite` (`:431`) im Audioblock, `PluginProcessor.cpp:947`.

**Station 5 — der Rollenwechsel nimmt den Aus-Weg weg.**
`PluginEditor.cpp:833-849` (`wechsleFlaecheWennNoetig`):

```
    juce::Component* legacy[] = {
        &glaettungWahl, &ansichtKnopf, &festhaltenKnopf, &vergleichWegKnopf,
        &resetKnopf, &hinweisKnopf, &messpunktKnopf, &konfliktKnopf,
        &markierungAusKnopf
    };
    for (auto* c : legacy)
        c->setVisible (! sollMain);
```

`markierungAusKnopf` ist Element neun (`:843`) und wird bei `:846` unsichtbar. Ein
`processor.markierungAus()` steht in dieser Funktion nicht.

**Station 6 — und die Sicherheitsnetze hören auf zu ticken.**
`PluginEditor.cpp:242-257`:

```
void EqCopilotEditor::timerCallback()
{
    wechsleFlaecheWennNoetig();
    if (mainFlaecheAktiv)
    {
        processor.sourcesTick();
        ...
        return;
    }
```

Der Rücksprung bei `:257` liegt **vor** allen drei Rücknahmen, `PluginEditor.cpp:270-278`:

```
    if (processor.markierungKillGemeldet() && markModus != MarkierungsModus::aus)
        markierungBeenden (u8 ("Markierung beendet — Offline-Render/Freilauf erkannt."));
    if (markModus != MarkierungsModus::aus)
    {
        if (processor.holeSamplerate() != markEngageSr)
            markierungBeenden (u8 ("Markierung beendet — Samplerate gewechselt."));
        else if (juce::Time::getMillisecondCounter() - letzteInteraktionMs > 10u * 60u * 1000u)
            markierungBeenden (u8 ("Markierung nach 10 Minuten ohne Bedienung beendet."));
    }
```

Freilauf-/Kill-Meldung, Sampleratenwechsel und der 10-Minuten-Totmann laufen in der
Main-Fläche nie.

**Station 7 — der einzige verbleibende Rückweg.**
`PluginEditor.cpp:233-240`, der Editor-Destruktor:

```
EqCopilotEditor::~EqCopilotEditor()
{
    processor.markierungAus();
    processor.setzeEditorOffen (false);
```

Das Schließen des Fensters beendet die Markierung — und `editorOffen` ist ohnehin ein
Faktor in `erlaubt` (`PluginProcessor.cpp:880`). Der Zustand ist also nicht unbegrenzt
klemmend, aber er ist ohne sichtbaren Handgriff nur durch Fensterschluss auflösbar.

### c) Zusage wörtlich

**Verletzt.** `CLAUDE.md:51-52`:

> „**Keine toten UI-Elemente** (User 24.08.2026): jedes sichtbare Element führt einen
> Handgriff aus oder meldet ehrlich einen Zustand."

Beide Hälften des Satzes sind an dieser Stelle gebrochen: in der Legacy-Fläche führt der
Solo/Puls-Handgriff nichts aus **und** meldet einen Zustand, den es nicht gibt
(`PluginEditor.cpp:807-812`); in der Main-Fläche gibt es einen Zustand, den kein
sichtbares Element mehr meldet oder beenden kann (`:843`, `:846`, `:257`).

Die Registerzeile **NAK-179** (`docs/offene-punkte.md:205`) beschreibt einen verwandten,
aber anderen Fall: „Der Hörmarker ist ohne gepatchte Hostbrücke stumm, und die UI sagt
nicht, warum." Dort fehlt die Brücke; hier ist die Brücke gleichgültig, weil die
Klassifikation sperrt. Phase 07 hat das getrennt (`roh/phase-07-modell.md:32`), und die
Trennung trägt am HEAD: `istMainKlassifiziert` (`PluginProcessor.cpp:874`) steht
unabhängig neben `echtzeitOk` (`:875`).

### d) Heutige Abdeckung

- **Bein A3 `EqCopMarkierungTest`** (`tools/beweise.ps1:503-508`) ist das einzige Bein zur
  Hörmarkierung. Sein Aufbauhelfer klassifiziert direkt als Main,
  `eq-copilot/plugin/tests/MarkierungTestMain.cpp:179-185`:

  ```
  static bool alsMainKlassifizieren (EqCopilotProcessor& p)
  {
      p.setzeEditorOffen (true);
      p.setzeBindung ("hub", {}, {});
      return p.holeKlassifikation() == nakama::state::Klassifikation::main;
  }
  ```

  Der Test fährt also nie die Legacy-Lage und bedient die Oberfläche nicht — er ruft den
  Prozessor direkt. Seine Behauptungszeile (`tools/beweise.ps1:508`) spricht von Render,
  Freilauf, U10-Term und Endlichkeit; über Klassifikationswechsel oder Bedienelemente steht
  nichts darin.
- **Bein B15 `EqCopShot`** (`:746`) rendert die Main-Fläche bei 760×430 und die
  Sonde012-Zustände; die Legacy-Fläche mit Befundliste und Markierungsknopf steht nicht in
  seiner Aufzählung.
- **Kein Bein fährt `wechsleFlaecheWennNoetig` mit aktivem Marker.** Die Funktion wird
  außerhalb des Editors nicht aufgerufen (`PluginEditor.cpp:229`, `:244`).
- **Register:** NAK-179 (`docs/offene-punkte.md:205`), als Produktfrage zur fehlenden
  Zustandsmeldung — nicht als Klassen-/Flächenfehler.
- **Irreführender Kommentar:** `PluginEditor.cpp:268-269` — „Hör-Markierung:
  Sicherheitsnetz-Ticks (Konzept v2 §4/N2) — Freilauf-Meldung löst den Latch sichtbar,
  Samplerate-Wechsel und Totmann ebenso." Der Satz beschreibt drei Netze, die in der
  Main-Fläche wegen des Rücksprungs bei `:257` nicht gespannt sind. Er steht heute
  vierzehn Zeilen unter dem Rücksprung, der ihn außer Kraft setzt.

### e) Urteil

**BESTÄTIGT**, mit drei Präzisierungen, die den Befund für die Reparatur aufteilen.

1. **Es sind drei trennbare Defekte in einer Zeile**, und nur der erste braucht die
   Legacy-Fläche: (i) der Statustext `:807-812` lügt in Legacy; (ii) der Flächenwechsel
   `:833-849` versteckt den Aus-Knopf, ohne den Auftrag zu beenden; (iii) der Rücksprung
   `:257` stellt die drei Sicherheitsnetze `:270-278` in der Main-Fläche ab. (iii) trifft
   **jeden** Marker in der Main-Fläche, nicht nur den aus Legacy geerbten — der Befund ist
   also breiter, als die Auditzeile sagt.
2. **Ein Rückweg bleibt**, und er gehört in den Befundtext: der Editor-Destruktor
   (`:237`) ruft `markierungAus()`. Ein Marker klemmt also nicht über das Schließen des
   Fensters hinaus. Das senkt den Schaden, hebt aber die Verletzung nicht auf: der User
   muss das Fenster schließen, um etwas zu beenden, das ihm niemand anzeigt.
3. **In Legacy ist der Aus-Knopf sichtbar und funktioniert** (`:843` steht in der
   Legacy-Liste, `markierungBeenden` bei `:817-826` ruft `processor.markierungAus()`
   bei `:821`). Der User kann den nie wirksamen Marker also abschalten — nur weiß er
   nicht, dass er nie wirkte.

### f) Kategorievorschlag

**DEFEKT** — `CLAUDE.md:51-52` ist eine datierte Userentscheidung und wird an drei Stellen
gebrochen.

**Schwere high**, wie das Audit. Kein Speicherfehler und kein Datenverlust, aber eine
sichtbare Fläche, die über den Klang des Masters etwas Falsches behauptet, und ein
Audiozustand ohne sichtbaren Ausschalter — auf einem Master mit laufender Wiedergabe.

### g) Kleinster Fix

Drei kleine, unabhängig prüfbare Schritte an drei Zeilen.

1. **Ehrliche Meldung statt stiller Sperre.** `schalteMarkierung`
   (`PluginEditor.cpp:754`) fragt vor `:800` denselben Term, den das Audio fragt — also
   `processor.holeKlassifikation() == Klassifikation::main`. Ist er falsch, wird nichts
   eingereicht, und `statusMeldung` sagt, was los ist („Einfärben braucht die
   Main-Rolle."). Der Text bei `:807-812` beschreibt dann nur noch die Lage, die wirklich
   eintritt.
2. **Der Flächenwechsel beendet, was er verbirgt.** `wechsleFlaecheWennNoetig`
   (`:833`) ruft vor dem `setVisible`-Lauf (`:845-846`) `markierungBeenden` mit einem
   Grund auf. `markierungBeenden` (`:817`) hat den Frühausstieg für „gar nicht aktiv"
   schon (`:819-820`), kostet also im Normalfall nichts.
3. **Die Sicherheitsnetze überleben den Rücksprung.** Der Block `:270-278` wandert vor die
   `if (mainFlaecheAktiv)`-Abzweigung bei `:245`, oder das `return` bei `:257` entfällt
   zugunsten eines `else`. Danach ticken Kill, Sampleratenwechsel und Totmann in beiden
   Flächen. Der Kommentar `:268-269` wird dann wieder wahr.

**Mitzuprüfende Beziehungen:** starten↔stoppen (`markierungEinreichen` bei `:800` gegen
`markierungAus` bei `:821` und `:237`), aktivieren↔abklingen — der weiche Ausfade ist
ausdrücklich gewollt (`HoerMarkierung.h:420-429`, NAK-47/M-34), und keiner der drei
Schritte darf ihn in einen harten Schnitt verwandeln; sowie öffnen↔schließen
(Editor-Destruktor `:233-240`).

**Was der Fix nicht ist:** keine neue Oberfläche. Schritt 1 ändert einen bestehenden
Statustext, Schritt 2 und 3 ändern Ablaufreihenfolgen. Der Designteil bleibt geparkt.

### h) Ort des Rotbeweises

**Bein A3 `EqCopMarkierungTest`** (`tools/beweise.ps1:503-508`), drei Fälle neben
`MarkierungTestMain.cpp:179-185`:

1. **Legacy meldet nicht falsch.** Prozessor mit offenem Editor, Rolle `sensor`
   (Klassifikation `legacy`), Marker einreichen, einen Block mit laufendem Transport
   fahren: Ausgang muss bitgleich zum Eingang sein — das gilt schon heute — **und** der
   Editorstatus darf nicht „nur dieser Bereich spielt" tragen. **Heute rot** an der
   zweiten Hälfte.
2. **Der Rollenwechsel entwaffnet.** Marker in Legacy einreichen, dann
   `setzeBindung ("hub", …)`, dann Blöcke fahren: der Ausgang muss bitgleich bleiben, bis
   der User in der Main-Fläche etwas tut. **Heute rot** — `zielAn`
   (`HoerMarkierung.h:493-499`) wird wahr, sobald `erlaubt` (`PluginProcessor.cpp:874`)
   kippt.
3. **Das Totmann-Netz gilt in beiden Flächen.** Marker in der Main-Fläche aktiv,
   Samplerate wechseln: die Markierung muss enden. **Heute rot** — `timerCallback` kehrt
   bei `PluginEditor.cpp:257` vorher zurück.

**Die Mutationen, die die Zusagen brechen**, sind `PluginEditor.cpp:846`
(`c->setVisible (! sollMain);` für `markierungAusKnopf`) und `PluginEditor.cpp:257`
(das `return;` im Main-Zweig). An diesen beiden Zeilen fällt der Rotbeweis, nicht an einem
Nebeneffekt: sie tragen die Zusage „jedes sichtbare Element … meldet ehrlich einen
Zustand" für den Marker. Die Behauptungszeile `tools/beweise.ps1:508` ist um einen Satz
über Klassifikation und Flächenwechsel zu ergänzen.

---

## 5. T3-14-01 — das Label für A landet unter B (W23)

*Befundzeile: `BEFUNDE.md:588` · Herleitung: `roh/phase-14-blindspots.md:35-45` ·
Blindspot-Zeile: `roh/phase-14-blindspots.md:19` · Beleggrenze des Nachtrags:
`BEFUNDE.md:584` („nicht dynamisch reproduziert", nicht durch Phase 10/11 gelaufen) ·
Arbeitsübersicht: `BEFUNDE.md:826`*

### a) Behauptung des Audits

Beim Wechsel der Quelle kann ein noch fokussiertes Label-Feld den Text der vorherigen
Quelle unter dem neuen Hauptziel speichern (`BEFUNDE.md:588`).

### b) Quellkette am HEAD

**Station 1 — ein Feld für alle Quellen.** `eq-copilot/plugin/src/PluginEditor.cpp:198`
begrenzt es auf 120 Zeichen; `:209-210` verdrahten beide Wege zum Commit:

```
    sourcesLabelFeld.onReturnKey = [this] { uebernehmeSourcesLabel(); };
    sourcesLabelFeld.onFocusLost = [this] { uebernehmeSourcesLabel(); };
```

**Station 2 — der Klick wechselt nur das Ziel.** `PluginEditor.cpp:974-988`
(`mouseDown`), Wirkung bei `:984`:

```
            processor.waehleSourcesHauptziel (sourcesAnzeige.quellen[i].instanceId);
```

Der Text bleibt unberührt; die Quellenzeilen sind gezeichnete Rechtecke
(`sourcesZeilen()`, `:951-967`), keine fokussierbaren Komponenten — der Klick nimmt dem
Feld den Tastaturfokus deshalb nicht.

**Station 3 — der Timer zieht das Aktionsziel auf B nach.**
`PluginEditor.cpp:249-253` ruft `aktualisiereSourcesSteuerung()`; dort `:904-909`:

```
    const auto vorherigesZiel = sourcesAktionsZiel;
    const auto it = std::find_if (sourcesAnzeige.quellen.begin(),
                                  sourcesAnzeige.quellen.end(),
        [] (const auto& q) { return q.hauptziel; });
    sourcesAktionsZiel = it == sourcesAnzeige.quellen.end()
                           ? std::string() : it->instanceId;
```

**Station 4 — und lädt B's Label ausdrücklich nicht nach, solange das Feld Fokus hat.**
`PluginEditor.cpp:934-936`:

```
    if (hatZiel && ! sourcesLabelFeld.hasKeyboardFocus (true)
        && sourcesLabelFeld.getText() != it->userLabel)
        sourcesLabelFeld.setText (it->userLabel, juce::dontSendNotification);
```

Ab hier zeigt die Zeile von B die Auswahlmarkierung, und das Feld daneben zeigt A's Text.

**Station 5 — der Commit nimmt Ziel und Text aus verschiedenen Leben.**
`PluginEditor.cpp:874-899` (`uebernehmeSourcesLabel`), die drei entscheidenden Zeilen:

```
:878     const auto ziel = sourcesAktionsZiel;                       // = B
:885     const auto label = sourcesLabelFeld.getText().substring (0, 120);   // = A-Text
:888     if (processor.benenneSourcesHauptziel (ziel, label))
```

Der Abbruch bei `:886-887` (`if (label == it->userLabel) return;`) vergleicht gegen
**B's** gespeichertes Label; bei verschiedenen Namen greift er nicht.

**Station 6 — die Prozessorwache ist korrekt und kann nichts merken.**
`eq-copilot/plugin/src/prozessor/Ipc.cpp:1205-1246`. Sie prüft dreifach, dass B das
aktuelle Hauptziel ist (`:1207`, `:1218`), was zutrifft, und schreibt dann `:1224`:

```
            gefunden->label = label;
```

Danach Publikation (`:1240-1241`) und Host-Dirty über
`werteSourcesPublikationAus` → `meldeHostDirty()` bei `Ipc.cpp:1489`.

**Station 7 — Zahlenrand: ein leeres Feld löscht B's Namen.** `:885` begrenzt nach oben,
nicht nach unten; `benenneSourcesHauptziel` verlangt nur `label.length() > 120` als
Ablehnung (`Ipc.cpp:1207`), und der Vertrag erlaubt den leeren String ausdrücklich
(`eq-copilot/schemas/state/nakama-state-v2.md:107-108`: „dem zugehörigen User-Label
(String, auch leer, höchstens 120 Unicode-Codepoints)"). Hatte A keinen Namen und B einen,
wird B's Name beim Bestätigen **gelöscht** — derselbe Weg, ohne dass der User überhaupt
Text gesehen hätte. Am oberen Rand ist das Verhalten harmlos: 120 Codepoints werden
zweimal gleich beschnitten (`:198` als Eingabebeschränkung, `:885` als Kappung).

### c) Zusage wörtlich

**Die vom Audit zitierte Zusage ist nicht gebrochen.** `docs/beweise/SONDE-012.md:231`,
U02:

> „Zu jedem sichtbaren Modellstand gibt es genau **ein Hauptziel** der Detailfläche; alle
> anderen Quellen sind klar als Referenzen benannt. … Auswahl oder Wegfall wird als ein
> Modellwechsel sichtbar; die UI zeigt nie zwei Hauptziele oder eine Referenz als
> Besitzer."

Es gibt durchgehend genau ein Hauptziel, und keine Referenz wird Besitzer. U03
(`SONDE-012.md:232`) hält ebenfalls, wörtlich:

> „Jede ausführbare Aktion adressiert ausschließlich die `instance_id` des beim Auslösen
> aktuellen Hauptziels."

Genau das tut `Ipc.cpp:1207/:1218`. Der Audit sagt selbst: „U03 allein schützt nicht vor
dem Text des vorigen Ziels" (`BEFUNDE.md:588`).

**Gebrochen ist eine andere Zusage: `CLAUDE.md:51-52`**

> „jedes sichtbare Element führt einen Handgriff aus oder meldet ehrlich einen Zustand."

Das Textfeld zeigt A's Entwurf, während die Auswahlmarkierung B zeigt und der Commit nach
B geht. Es meldet einen Zustand — „dies ist der Name dieser Quelle" —, den es nicht gibt.
Dieselbe Zusage trägt auch T3-07-02 (Abschnitt 4).

### d) Heutige Abdeckung

- **Bein B13 `EqCopSonde012SourcesModelTest`** (`tools/beweise.ps1:744`) misst U03 an
  Ziel-IDs: `eq-copilot/plugin/tests/Sonde012SourcesModelTest.cpp:460`
  (`actions_address_only_current_main_target_across_target_change`). Ein fokussierter
  Textentwurf kommt darin nicht vor — das Modell kennt kein Textfeld.
- **Bein B14** (`:745`) prüft bei
  `eq-copilot/plugin/tests/Sonde012ProjectReloadTest.cpp:2674` das deaktivierte Feld des
  nicht führenden Main — der Gegenfall zum Schreiben, nicht zur Textherkunft.
- **Bein B15 `EqCopShot`** (`:746`) rendert statische Darstellungen
  (`eq-copilot/plugin/tests/ShotTestMain.cpp:199`); ein Shot kann eine Fokusfolge nicht
  fahren.
- **Register:** keine Zeile.
- **Irreführender Kommentar:** keiner. `PluginEditor.cpp:934-936` trägt gar keinen — und
  genau dort wäre einer nötig gewesen, denn die Zeile trifft bewusst die Entscheidung,
  einen Entwurf zu erhalten, ohne zu regeln, wozu er gehört.

### e) Urteil

**PRÄZISIERT.** Die Mechanik der Auditzeile trägt vollständig und ist am HEAD Zeile für
Zeile nachvollziehbar. Zwei Dinge sind anders:

1. **Die zitierte Zusage ist die falsche.** U02 und U03 (`SONDE-012.md:231`, `:232`)
   halten beide. Wer den Befund als „U02 gebrochen" führt, wird ihn beim ersten Widerspruch
   verlieren. Die tragende Zusage ist `CLAUDE.md:51-52`.
2. **Der schärfste Fall ist der leere Entwurf.** Ist A unbenannt und B benannt, löscht die
   Bestätigung B's Namen (`PluginEditor.cpp:885` gegen `Ipc.cpp:1207`,
   `nakama-state-v2.md:107`). Der Audit beschreibt den Fall „verschiedene Labels"; der
   Löschfall ist derselbe Weg mit größerem Schaden und muss in den Rotbeweis.

### f) Kategorievorschlag

**DEFEKT** — `CLAUDE.md:51-52` ist eine datierte Userentscheidung, und das Feld meldet
einen unwahren Zustand.

**Schwere medium**, wie das Audit. Persistente Änderung mit Host-Dirty an einer falschen
Quelle, die der User in seinem Projekt wiederfindet — aber kein Audiofehler, kein
unlesbarer State, und ein erneutes Tippen stellt beide Namen wieder her.

### g) Kleinster Fix

Den Entwurf an die Quelle binden, für die er begonnen wurde.

1. Der Editor führt neben `sourcesAktionsZiel` (`PluginEditor.h`, gesetzt bei
   `PluginEditor.cpp:908`) eine zweite Kennung: die Quelle, für die der aktuelle
   Feldinhalt gilt. Sie wird gesetzt, wenn `:936` den Text lädt.
2. `uebernehmeSourcesLabel` (`:874`) vergleicht bei `:878` beide. Stimmen sie nicht
   überein, wird nicht geschrieben.
3. `aktualisiereSourcesSteuerung` (`:902`) entscheidet beim Zielwechsel ausdrücklich, was
   mit dem alten Entwurf geschieht. Der kleinste ehrliche Weg: den Entwurf für die alte
   Quelle abschließen (also noch unter **A** schreiben, wo er hingehört) und dann B's Label
   laden — dann verliert der User nichts, und nichts landet falsch. Das erfordert, dass
   `:934` beim Zielwechsel nicht mehr am Fokus scheitert, sondern am Kennungsvergleich.

**Mitzuprüfende Beziehungen:** speichern↔laden — der geschriebene Name muss durch
Save/Load unverändert zurückkommen (B14, `tools/beweise.ps1:745`); starten↔stoppen der
Bearbeitung — `onReturnKey` und `onFocusLost` (`:209-210`) sind zwei Eingänge in denselben
Commit und müssen dieselbe Regel sehen, dazu Escape und der Wegfall der Quelle während der
Bearbeitung.

**Keine neue Oberfläche.** Der Fix ändert Bedingungen an drei bestehenden Stellen; das Feld
bleibt, wo es ist.

### h) Ort des Rotbeweises

**Bein B13 `EqCopSonde012SourcesModelTest`** (`tools/beweise.ps1:744`) trägt heute U03,
kennt aber kein Textfeld; der Fall braucht einen echten Editor und gehört deshalb zu
**B15 `EqCopShot`** (`:746`) oder in ein Editor-Bein der B13/B14-Familie. Der Fall:

1. Führender Main, zwei bestätigte Quellen A und B mit **verschiedenen** Labels.
2. Hauptziel A, Feld fokussieren, Text ändern (oder gar nicht ändern — beides muss geprüft
   werden).
3. B in der gezeichneten Liste anklicken (`PluginEditor.cpp:984`), einen Timertick fahren
   (`:242`), dann Enter (`:209`).
4. Messen: A's Label, B's Label und die Zahl der Dirty-Meldungen.

**Erfolgskriterium:** B's Label ist unverändert; A's Label trägt den Entwurf oder ist
unverändert, je nach gewählter Regel; die Dirty-Zahl passt zur Zahl der wirklich
geänderten Namen. **Heute rot:** B trägt A's Text und meldet Dirty.

**Zweiter Fall, der heute mehr kaputt macht:** A **ohne** Label, B **mit** Label, sonst
gleiche Folge. Heute wird B's Name gelöscht.

**Gegenfälle, die grün bleiben müssen:** Escape statt Enter; Fokusverlust durch Klick
außerhalb der Liste; unveränderter Text bei unverändertem Ziel (darf keine Mutation und
kein Dirty erzeugen — `Ipc.cpp:1221` hält das heute); ein Label mit 120 Codepoints, das
zweimal gleich beschnitten wird (`:198` und `:885`).

**Die Mutation, die die Zusage bricht**, ist `PluginEditor.cpp:934`
(`! sourcesLabelFeld.hasKeyboardFocus (true)`) — die Bedingung, die den fremden Entwurf
stehen lässt. Nach dem Fix trägt der Kennungsvergleich die Zusage, und die Mutation fällt
dort. Die Behauptungszeile `tools/beweise.ps1:744` ist um einen Satz über die Herkunft des
Textentwurfs zu ergänzen.

---

## 6. T3-07-05 — akzeptierte Quellen ab 21 sind nicht erreichbar (W24)

*Befundzeile: `BEFUNDE.md:440` · Herabstufung Phase 10: `BEFUNDE.md:53` und `:537`,
Begründung `:106` · Skeptikerpass: `roh/phase-07-modell.md:35` · Handgriffkarte:
`roh/phase-07-modell.md:218` · Kartenentwurf PF11-17: `BEFUNDE.md:323-328` ·
Arbeitsübersicht: `BEFUNDE.md:827`*

### a) Behauptung des Audits

Ab Quelle 21 fehlen Auswahl und quellenbezogene Handgriffe: das Modell akzeptiert bis zu 64
Mitglieder, die feste Main-Fläche stellt höchstens 20 anklickbare Zeilen her
(`BEFUNDE.md:440`); Phase 10 hat das von DEFEKT auf **LÜCKE** herabgestuft, weil die
zitierte Zusage nur 16 sichtbare Probes verspricht (`BEFUNDE.md:537`).

### b) Quellkette am HEAD

**Station 1 — die Fläche ist fest.** `eq-copilot/plugin/src/PluginEditor.cpp:851-858`:

```
    if (sollMain)
    {
        // Die abgenommene funktionale Pruefflaeche hat genau dieses Mass.
        // Keine daraus abgeleitete Stil- oder Responsiventscheidung.
        if (auto* c = getConstrainer())
            c->setFixedAspectRatio (0.0);
        setResizable (false, false);
        setSize (760, 430);
```

**Station 2 — die Spalte und ihre Zeilen.** `PluginEditor.cpp:946-967`:

```
juce::Rectangle<int> EqCopilotEditor::sourcesSpalte() const
{
    return { 0, 0, juce::jmin (310, getWidth()), getHeight() };
}

std::vector<juce::Rectangle<int>> EqCopilotEditor::sourcesZeilen() const
{
    std::vector<juce::Rectangle<int>> aus;
    if (sourcesAnzeige.quellen.empty())
        return aus;
    auto r = sourcesSpalte().reduced (8);
    r.removeFromTop (48);
    const int h = juce::jlimit (18, 34,
        r.getHeight() / juce::jmax (1, (int) sourcesAnzeige.quellen.size()));
    for (size_t i = 0; i < sourcesAnzeige.quellen.size(); ++i)
    {
        if (r.getHeight() < h)
            break;
        aus.push_back (r.removeFromTop (h));
    }
    return aus;
}
```

**Die Rechnung, hier selbst nachgerechnet.** Bei 760×430 ist die Spalte 310×430;
`reduced(8)` macht daraus 294×414; `removeFromTop(48)` lässt **366 px** übrig. Daraus
`h = jlimit(18, 34, 366/n)` und die Zahl der Zeilen `floor(366/h)`:

| Quellen n | h in px | erzeugte Zeilen | alle erreichbar |
|---:|---:|---:|---|
| 16 | 22 | 16 | ja |
| 20 | 18 | 20 | ja |
| 21 | 18 (unterer Anschlag) | 20 | **nein** |
| 32 | 18 (unterer Anschlag) | 20 | **nein** |
| 64 | 18 (unterer Anschlag) | 20 | **nein** |

Die Kante liegt bei `366/n < 18`, also **ab n = 21** (`366/21 = 17`, geklemmt auf 18);
`20 × 18 = 360`, es bleiben 6 px, die Schleife bricht bei `:962-963` ab. Das deckt sich
mit der Auditrechnung `(430 − 2×8 − 48)/18 = 20`.

**Station 3 — dieselbe Liste trägt Zeichnung und Trefferprüfung.**
`PluginEditor.cpp:1135` (in `paint`) und `:980` (in `mouseDown`) rufen beide
`sourcesZeilen()`. Quelle 21 ist also **nicht nur unklickbar, sondern gar nicht gezeichnet**.

**Station 4 — die Kapazitäten als Zahlen.**
Modell: `eq-copilot/plugin/state/NakamaState.h:77`
(`inline constexpr int maxMainProjectMitglieder = 64;`), durchgesetzt in
`eq-copilot/plugin/src/SourcesModel.cpp:357-358` und `:463-464`.
Draht: `eq-copilot/schemas/v3/eq-ipc-v3.schema.json:1110-1112` — `"mitglieder"` mit
`"maxItems": 64`. Die Zusage: `docs/FL-Nakama-Sonden-Design-Entwurf.md:1590`
(„normale Projektgröße | UX für 1 Main + 16 sichtbare Sonden, Verträge und Lasttests bis
32") und `:4277` („Live-Landkarte für 1 Main + 16 sichtbare Probes, Lastvertrag bis 32").

**Station 5 — die Aktionsknöpfe finden auch eine ungezeichnete Quelle.**
`aktualisiereSourcesSteuerung` (`PluginEditor.cpp:902-944`) sucht das Hauptziel über die
**ganze** Liste (`:905-907`), nicht über die 20 Zeilen, und schaltet danach
`sourcesAktionKnopf` (`:919`) und `sourcesLabelFeld` (`:933`) sichtbar. Das Hauptziel muss
dabei nicht vom User stammen: `eq-copilot/plugin/src/SourcesModel.cpp:1772-1777`:

```
void SourcesModel::stelleZielSicher()
{
    if (! hauptziel.empty() && eintraege.count (hauptziel) != 0)
        return;
    hauptziel = eintraege.empty() ? std::string() : eintraege.begin()->first;
}
```

`eintraege` ist nach `instanceId` geordnet, die **Anzeige** dagegen nach Mixerindex, dann
Name, dann `instanceId` (`SourcesModel.cpp:1651-1663`). Der Ersatzfall kann damit an jeder
Anzeigeposition landen, auch jenseits der zwanzigsten. Dann stehen „Remove source" und das
Label-Feld sichtbar und scharf auf einer Quelle, die der User nirgends sieht.

### c) Zusage wörtlich

**Die Quellen schweigen zu Quellen oberhalb der sichtbaren Kapazität** — und die vom
Audit ursprünglich zitierte Zusage trägt den Defektvorwurf nicht. `docs/plan/plan.json`,
Schritt S18–19: „Gen Fläche 1 zeigt je Quelle den Fehlerzustand ehrlich." Der Satz ist mit
dem ausdrücklich begrenzten Sichtumfang zu lesen: der Entwurf fordert 16 sichtbare Probes
(`docs/FL-Nakama-Sonden-Design-Entwurf.md:1590`, `:4277`), nicht 64. Die 64 Plätze des
Modells (`NakamaState.h:77`) sind eine **Speicher**-, keine Sichtzusage. Phase 10 hat
daraus die Herabstufung auf LÜCKE abgeleitet (`BEFUNDE.md:106`), und das trägt am HEAD.

**Station 5 ist davon nicht gedeckt.** Ein sichtbarer, scharfer Knopf auf einer
unsichtbaren Quelle fällt unter `CLAUDE.md:51-52` („meldet ehrlich einen Zustand"). Das ist
ein eigener, kleinerer Punkt innerhalb derselben Reparatur.

### d) Heutige Abdeckung

- **Bein B15 `EqCopShot`** (`tools/beweise.ps1:746`) rendert laut Behauptungszeile „bei
  exakt 760x430: 0/1/16 Quellen". Die Fallliste bestätigt es:
  `eq-copilot/plugin/tests/ShotTestMain.cpp:115-118` — `sicht (0)`, `sicht (1)`,
  `sicht (16)`, `sicht (1)`. **Keine 20, keine 21, keine 32, keine 64.** Die einzige
  Änderung an dieser Datei seit dem Auditstand ist ein Kopfkommentar (NAK-309).
- **Bein B13** (`:744`) fährt 16 und 32 Quellen — aber als **Zeitmessung** des Modells:
  `eq-copilot/plugin/tests/Sonde012SourcesModelTest.cpp:815-818`
  (`visible_latency_16_and_32_sources`, Kriterium p95 ≤ 300 ms). Die Behauptungszeile sagt
  das selbst („Die integrierte 16-/32-Quellen-Zeitmessung gehoert A23"). Erreichbarkeit
  misst sie nicht.
- **Kein Bein prüft `sourcesZeilen()`** gegen die Zahl der Quellen.
- **Register:** keine eigene Zeile; NAK-312 führt den Punkt (`docs/offene-punkte.md:316`).
- **Irreführender Kommentar:** `PluginEditor.cpp:853-854` — „Die abgenommene funktionale
  Pruefflaeche hat genau dieses Mass. Keine daraus abgeleitete Stil- oder
  Responsiventscheidung." Der Satz ist als Designaussage richtig und heute trotzdem
  irreführend: aus genau diesem Maß folgt die Zahl 20, und die ist keine Stilfrage,
  sondern die Bediengrenze. Wer die Fläche später auf die in `CLAUDE.md` genannte
  Standardgröße 950×538 bringt, bekommt automatisch 26 Zeilen
  (`(538 − 16 − 48)/18 = 26`). Die Zahl 20 darf deshalb nirgends hart hingeschrieben
  werden.

### e) Urteil

**BESTÄTIGT** in der Fassung nach der Phase-10-Herabstufung, mit zwei Präzisierungen.

1. **„Nicht erreichbar" ist zu schwach: die Quellen ab 21 sind nicht einmal gezeichnet.**
   `paint` (`:1135`) und `mouseDown` (`:980`) teilen sich `sourcesZeilen()`. Der User sieht
   nicht, dass etwas fehlt — es gibt keinen Rand, keinen Zähler, keine abgeschnittene
   Zeile. Das ist für die Formulierung der Karte wichtig.
2. **Der gefährlichere Teil steht nicht in der Auditzeile:** die Aktionsknöpfe können eine
   ungezeichnete Quelle adressieren (`:905-907`, `:919`, `:933` gegen
   `SourcesModel.cpp:1772-1777` und den abweichenden Anzeigesort `:1651-1663`). Das ist
   kein „fehlender Handgriff", sondern ein **wirksamer Handgriff auf ein unsichtbares
   Ziel** — die Umkehrung des Befundes und sein schärferer Fall.

Die Zahlenränder sind am HEAD selbst nachgerechnet (Tabelle unter b): 16 und 20 sind
vollständig erreichbar, ab 21 bleiben es genau 20.

### f) Kategorievorschlag

**LÜCKE** — die Quellen sagen nichts über akzeptierte Quellen oberhalb der sichtbaren
Kapazität; die zugesagten 16 sind erfüllt.

**Schwere medium**, wie Phase 10. Erreichbar nur in Projekten mit mehr als zwanzig
Probeeq-Instanzen — heute ungewöhnlich —, aber dann unsichtbar und ohne Rückweg, und mit
dem Punkt aus Präzisierung 2 auch mit einer wirksamen Aktion auf ein blindes Ziel.

### g) Kleinster Fix

Der Weg hängt an U51; beide Richtungen sind klein und liegen im Modell, nicht in der
Fläche.

**Bei „Annahme begrenzen" (Weg 1, Empfehlung der Karte):** Die Annahmegrenze wandert vom
Speicherdeckel 64 (`NakamaState.h:77`) auf die Zahl der Zeilen, die die Fläche wirklich
herstellt, und diese Zahl wird **aus der Geometrie gerechnet**, nicht hingeschrieben — die
Fläche kennt sie schon (`PluginEditor.cpp:951-967`). Der Ort der Ablehnung ist
`SourcesModel` beim Aufnehmen einer neuen Quelle; die Ablehnung braucht einen Grund, der
über den vorhandenen Diagnosekanal lesbar ist (`SourcesModel::Diagnose`, benutzt bei
`PluginEditor.cpp:916-918`, `:940-943`). **Wichtig für die Erwartung:** die sichtbare
Meldung „Liste voll" existiert noch nicht; „ehrlich melden" heißt heute, dass das Modell
den Grund führt, die Anzeige folgt mit der Oberfläche.

**Bei „alle bis 64 erreichbar halten" (Weg 2):** Das braucht einen Blätter- oder
Scrollweg für die Quellenspalte. Ein `juce::Viewport` liegt schon im Editor, gehört aber
ausschließlich den Legacy-Befundkarten (`PluginEditor.cpp:527-531`); ihn zu übernehmen ist
Oberflächenarbeit und damit **geparkt**. Vor der Antwort ist dieser Weg nicht baubar.

**In beiden Fällen, und ohne Antwort baubar:** Präzisierung 2 schließen. Entweder findet
`aktualisiereSourcesSteuerung` (`:902-909`) das Hauptziel nur unter den gezeichneten
Zeilen, oder `stelleZielSicher` (`SourcesModel.cpp:1772-1777`) wählt seinen Ersatz nach
derselben Ordnung, in der die Fläche zeichnet (`:1651-1663`). Die zweite Variante ist die
richtige: dann ist das Ersatzziel immer die erste **angezeigte** Quelle, und die Fläche
kann nie auf etwas zeigen, das sie nicht malt.

**Mitzuprüfende Beziehungen:** verbinden↔trennen (Annahme und Wegfall einer Quelle müssen
dieselbe Grenze sehen) und speichern↔laden — **vorhandener State wird nie zur Anpassung an
eine UI-Grenze gelöscht.** Ein Projekt mit 40 gespeicherten Mitgliedern muss seine 40
behalten (`NakamaState.cpp:1536-1580`, Deckel `maxMainProjectMitglieder`); begrenzt wird
die **Annahme neuer Quellen**, nicht der Bestand. Der Kartenentwurf PF11-17 sagt dasselbe
(`BEFUNDE.md:328`).

### h) Ort des Rotbeweises

**Bein B15 `EqCopShot`** (`tools/beweise.ps1:746`), neue Fälle neben
`ShotTestMain.cpp:115-118`: `sicht (20)`, `sicht (21)`, `sicht (32)`, `sicht (64)`.
Der Shot rendert bei exakt 760×430, also bei der Geometrie, aus der die Zahl 20 entsteht.

**Erfolgskriterium nach Weg 1:** die Zahl der angenommenen Quellen erreicht nie einen
Stand, in dem `sourcesZeilen().size() < sourcesAnzeige.quellen.size()` gilt; die
abgelehnte Quelle trägt einen lesbaren Grund. **Erfolgskriterium nach Weg 2:** jede
angenommene Quelle ist über einen Bedienweg erreichbar. **Heute rot in beiden Fällen** —
bei 21, 32 und 64 Quellen erzeugt `sourcesZeilen()` genau 20 Rechtecke, und keine
Ablehnung wird gemeldet.

**Die Mutation, die die Zusage bricht**, sind die zwei Zeilen
`eq-copilot/plugin/src/PluginEditor.cpp:958-959`:

```
    const int h = juce::jlimit (18, 34,
        r.getHeight() / juce::jmax (1, (int) sourcesAnzeige.quellen.size()));
```

Der untere Anschlag 18 ist die Zeile, die die Grenze trägt — nicht der Abbruch bei `:962`,
der nur ihre Folge ist. Ein Rotbeweis, der die Zeilenzahl direkt prüft, fällt also an der
richtigen Stelle.

**Zweiter Fall (Präzisierung 2), ohne Produktantwort baubar:** 25 Quellen, Hauptziel per
`stelleZielSicher` (`SourcesModel.cpp:1776`) vergeben, Anzeigeposition des Hauptziels
messen. **Erfolgskriterium:** das Hauptziel liegt unter den gezeichneten Zeilen, oder die
Aktionsknöpfe sind nicht sichtbar. **Heute rot**, sobald die `instanceId`-Ordnung von der
Anzeigeordnung abweicht.

**Gegenfälle, die grün bleiben:** 0, 1 und 16 Quellen wie heute (die drei bestehenden
Shots, `:115-117`); 20 Quellen vollständig erreichbar; ein geladener Stand mit mehr
Mitgliedern als die Fläche zeigt, behält nach Save/Load alle Mitglieder (Gegenwache Bein
**B14**, `tools/beweise.ps1:745`). Die Behauptungszeile `:746` ist um die
Erreichbarkeitsaussage zu ergänzen, die heute nur „0/1/16 Quellen" nennt.

---

## 7. Trennung Technik und Produkt

| ID | Was der Dirigent als Technikregel entscheidet | Was nur der User entscheidet | Was unabhängig von der Antwort baubar ist |
|---|---|---|---|
| T3-04-01 | Alles: Schleusenmitglied und seine Deklarationsposition, Ort des Schließens, Form der Testzugänge, Probe-Pipename, Kanarienvogel im Rotbeweis. | Nichts. R-D2 (`NAK-246.md:1008`) ist eine datierte Entscheidzeile ohne Vorbehalt; sie gilt für Produkt-Callbacks, und diese drei sind welche. | Alles unter 1 g) und 1 h). |
| T3-04-03 | Alles: Besitzform (SafePointer über den Editor), Ort der Lebendprüfung, Testform des modalen Abbaus, berichtigter Kommentar. | Nichts. Es geht um Speicherlebensdauer, nicht um Verhalten: der Handgriff bleibt derselbe, er fasst nur nichts Totes mehr an. | Alles unter 2 g) und 2 h). |
| T3-02-06 | Ort der Ablage (eigenes versioniertes Kind gegen Verwerfen), Schema- und Migrationsform, Fixtures, Testform, Reihenfolge gegenüber W20. | **Ja:** ob der Projektinhalt den Rollenwechsel überlebt. Das ändert, was ein Musiker nach „Messpunkt und zurück" in seinem Projekt vorfindet — es ist Produktverhalten, kein Implementierungsweg. **Karte U49.** | Der Rotbeweis aus 3 h) in beiden Richtungen; die Messung des heutigen Verhaltens; die Berichtigung des Kartentexts. |
| T3-07-02 | Alles: wo die Klassifikation abgefragt wird, ob der Flächenwechsel beendet oder verweigert, wo die drei Netze ticken, Wortlaut des Statustexts. | Nichts. `CLAUDE.md:51-52` ist die datierte Userentscheidung vom 24.08.2026 und beantwortet die Frage schon. | Alles unter 4 g) und 4 h). |
| T3-14-01 | Alles: Form der Entwurfskennung, ob der alte Entwurf abgeschlossen oder verworfen wird, Testform. Die Wahl „abschließen oder verwerfen" ändert nichts an dem, was der User sieht, solange nichts an der falschen Quelle landet. | Nichts. Dieselbe Entscheidzeile wie bei T3-07-02. | Alles unter 5 g) und 5 h). |
| T3-07-05 | Ort der Grenze (Modell), wie die Zeilenzahl gerechnet wird, Form der Ablehnungsmeldung, Ordnung des Ersatzziels, Testform. | **Ja:** ob eine 21. Probeeq angenommen und unsichtbar wird oder ehrlich abgewiesen. Das ändert, was passiert, wenn ein Musiker eine weitere Instanz aufmacht. **Karte U51.** | Präzisierung 2 vollständig (Ersatzziel nach Anzeigeordnung, 6 g) letzter Absatz); der Shot-Fall mit 20 Quellen als Gegenwache; die Messung der heutigen Zeilenzahl. |

---

## 8. Karten U49 und U51

### U49 — „Rollenwechsel einer Gen-Instanz: bleiben die gesammelten Daten erhalten?"

**Trifft den Befund, aber der Text ist in einem Punkt falsch und muss vor dem Stellen
berichtigt werden.**

Heutiger Text im Feld `was` (`docs/plan/fragen.json`, U49, seit 18.09.):

> „Eine Gen-Instanz kann vom Hauptprogramm zum reinen Messpunkt und zurück wechseln.
> Wechselt sie live, bleiben Quellen, Befunde und Vergleiche heute erhalten; nach Speichern
> und Neuladen sind sie weg — ein Widerspruch."

**Der Fehler:** „Wechselt sie live, bleiben Quellen … erhalten" stimmt nicht. Die
bestätigten Quellen sind `mainProjectMitglieder`, und `State.cpp:322` löscht sie im
Augenblick des Rollenwechsels, ohne Speichern. Erhalten bleiben live nur die sechs anderen
Sammlungen (`NakamaState.h:451`, `:456`, `:457`, `:458`, `:465`, `:471`) — Passagen,
Intents, Schutzangaben, Beziehungen, Bestandsrevision, Assistent.

**Vorschlag für `was`, in Alltagssprache:**

> „Eine Gen-Instanz kann vom Hauptprogramm zum reinen Messpunkt und zurück wechseln. Heute
> ist das Ergebnis uneinheitlich: die Liste deiner angemeldeten Messpunkte ist sofort weg,
> auch ohne Speichern; markierte Passagen, Rollen, Schutzangaben und der Assistentenstand
> überstehen den Wechsel, solange du nicht speicherst und neu lädst — danach sind auch sie
> weg. (1) Empfohlen: alles bleibt erhalten, in beiden Fällen gleich; gelöscht wird nur
> durch einen ausdrücklichen Handgriff. (2) Der Wechsel löscht alles bewusst, in beiden
> Fällen gleich — dann weißt du sicher, dass der Messpunkt nichts vom Hauptprogramm
> mitschleppt."

Das Feld `warum` bleibt inhaltlich richtig („Ohne Antwort baut der Dirigent Weg 1") und
deckt sich mit dem Default des Kartenentwurfs PF11-03 (`BEFUNDE.md:230`: „Bestände
erhalten; Löschen nur ausdrücklich, nicht als Nebenwirkung des Rollenwechsels"). Der Zusatz
„unbekannte gespeicherte Felder bleiben in jedem Fall erhalten" ist am HEAD gedeckt
(`nakama-state-v2.md:103`, additive unbekannte Eigenschaften).

**Zusatz, den der Dirigent im `warum` ergänzen sollte:** Weg 1 ist nicht gratis. Weil die
Kind-Matrix MainProject für `legacy` verbietet (`nakama-state-v2.md:100`), braucht
„erhalten über Speichern hinweg" ein neues, versioniertes Kind im Zustandsbaum — also
Schema, Leser, Schreiber, Fixtures und Migration in einem Satz. Das ist Technik und bleibt
beim Dirigenten, aber es macht Weg 1 zum größeren Ticket. Die Karte muss das nicht
erklären; der Planschritt muss es wissen.

### U51 — „Mehr als 20 Probeeq-Instanzen in einem Projekt: begrenzen oder erreichbar halten?"

**Trifft den Befund; die Zahlen im Text sind am HEAD korrekt.** „für 16 Zeilen zugesagt"
deckt sich mit `docs/FL-Nakama-Sonden-Design-Entwurf.md:1590` und `:4277`; „heute passen
20" deckt sich mit meiner eigenen Rechnung (Abschnitt 6 b); „intern nimmt Nakama bis zu 64
Quellen an" deckt sich mit `NakamaState.h:77` und `eq-ipc-v3.schema.json:1110-1112`.

**Zwei Dinge sind dem Dirigenten vor dem Stellen zu sagen.**

1. **Die Empfehlung der Karte steht gegen den Default des Auditentwurfs.** U51 empfiehlt
   Weg 1 („Annahme auf 20 begrenzen"), PF11-17 hat als Default Weg 2
   (`BEFUNDE.md:328`: „Angenommene Quellen erreichbar halten; vorhandenen State nie zur
   Anpassung an eine UI-Grenze löschen"). Beide Sätze widersprechen sich nicht vollständig
   — der zweite Halbsatz von PF11-17 gilt in jedem Fall —, aber die Vorzugsrichtung ist
   entgegengesetzt. Das gehört gesagt, sonst behauptet die Karte eine Einigkeit, die es
   nicht gibt. **Ich halte Weg 1 trotzdem für richtig**, und zwar aus einem technischen
   Grund, der in die Karte gehört: Weg 2 braucht eine blätterbare Liste, und die ist
   Oberflächenarbeit im geparkten Designteil. Weg 2 ist vor einer Designfreigabe nicht
   baubar; Weg 1 ist es.
2. **Der Satz „heute passen 20" sollte die Folge benennen.** Heute ist die 21. Quelle nicht
   „vorhanden, aber unbedienbar", sondern **unsichtbar**: sie wird nicht gezeichnet
   (`PluginEditor.cpp:1135` und `:980` teilen sich dieselbe Zeilenliste). Für die
   Entscheidung ist das der Unterschied zwischen „unbequem" und „du weißt nicht, dass sie
   da ist".

**Vorschlag für zwei Sätze im Feld `was`,** an Stelle von „Ab der 21. wäre eine Quelle zwar
angenommen, aber nicht bedienbar.":

> „Ab der 21. ist eine Quelle zwar angenommen, taucht in der Liste aber gar nicht mehr auf
> — du siehst nicht, dass sie fehlt. (2) … das braucht später eine blätterbare Liste und
> ist deshalb erst nach der Designarbeit baubar."

**Nicht zu ändern:** „Bereits gespeicherte Quellen werden in keinem Fall gelöscht." Der
Satz ist am HEAD haltbar und deckt sich mit PF11-17.

---

## 9. Gemeinsame Ursachen

**Wurzel 1 — Besitz eines Callbacks über das Ende seines Eigentümers hinaus.**
**T3-04-01** und **T3-04-03**. Beide sind derselbe Fehler in zwei Ausprägungen: ein
Rückruf hält eine rohe Referenz auf einen Prozessor, dessen Lebensdauer er nicht
kontrolliert. NAK-246 hat das Muster für Gens IPC-Callbacks gelöst
(`Schleuse.h`, `PluginProcessor.cpp:500`) und dabei zwei Nachbarn stehen lassen: die
Sonde-Hälfte und die JUCE-Modale. Phase 10 warnt zu Recht, sie nicht gleichzusetzen
(`roh/phase-10-skeptiker.md:122`: „T3-04-01 und T3-04-03 besitzen verschiedene
Lebensdauern") — die Reparaturen sind verschieden (Schleuse gegen SafePointer), die
**Prüfform** ist dieselbe: Callback vor dem Zugriff festhalten, Eigentümer abbauen,
Callback freigeben, Kanarienvogel lesen. Ein Bein, das diese Form einmal baut, trägt beide.

**Wurzel 2 — ein Zustandswechsel, der seine Folgen nicht mitnimmt.**
**T3-02-06** und **T3-07-02** hängen an derselben Funktion:
`EqCopilotProcessor::setzeBindung` (`State.cpp:287-363`). Sie ändert die Klasse
(`:320`), löscht eine Sammlung (`:322`), spiegelt die Klassifikation (`:344`) — und löst
damit zwei Folgen aus, die niemand behandelt: die anderen sechs Sammlungen behalten eine
Lebensdauer, die keine Regel kennt, und die Audio-Erlaubnis kippt, während die Fläche
umschaltet (`PluginEditor.cpp:833`). Beide Befunde sind Nachwirkungen **eines**
Handgriffs. Wer den Rollenwechsel als einen Satz aus State, Bedienung und Audio behandelt
— wie die Arbeitsübersicht es verlangt (`BEFUNDE.md:825`) —, findet beide auf demselben
Weg.

**Wurzel 3 — die Fläche und das Modell rechnen mit verschiedenen Mengen.**
**T3-14-01** und **T3-07-05**. Bei T3-14-01 gehören Text und Ziel zu verschiedenen
Quellen (`PluginEditor.cpp:878` gegen `:885`); bei T3-07-05 kennt das Modell 64 Quellen und
die Fläche zwanzig (`NakamaState.h:77` gegen `PluginEditor.cpp:958-959`), und die
Aktionssteuerung sucht in der einen Menge, während die Zeichnung die andere malt
(`:905-907` gegen `:1135`). Beide Male fehlt dieselbe Regel: **welche Menge ist die
Wahrheit der sichtbaren Aktion?** Beide Reparaturen setzen an derselben Funktion an,
`aktualisiereSourcesSteuerung` (`:902-944`).

**Berührung mit Teil A (W02, W05, W06).** Ich habe diese IDs nicht geprüft; die Berührung
steht deshalb als Hinweis, nicht als Befund.

- **W06 ↔ T3-07-02.** W06 führt T3-01-04, T3-01-09 und T3-01-10 — erstes Offline-Audio,
  geerbter Hostbypass, Gen-Reset bei aktiver Hörmarkierung (`docs/offene-punkte.md:316`).
  Der dritte trifft denselben Marker wie T3-07-02 und dieselben Abschaltwege. Der
  Rücksprung `PluginEditor.cpp:257` und die drei Netze `:270-278` gehören zu beiden. **Die
  beiden gehören in dieselbe Etappe**, sonst repariert eine die Rücknahme und die andere
  verlegt sie. Die Arbeitsübersicht nennt die Abhängigkeit schon (`BEFUNDE.md:825`:
  „W06 Hör-/Offlinezustand").
- **W02 ↔ T3-02-06.** W02 führt T3-05-01 und T3-05-02: eine alte Hostmailbox überschreibt
  geladenen Klang, read-only verarbeitet Hostparameter. Beide sagen dasselbe wie T3-02-06
  über eine andere Achse — **was gilt nach einem Ladevorgang?** Wer die Antwort auf U49
  umsetzt, berührt Save und Load; wer W02 repariert, ebenfalls. Der Konflikt ist
  vermeidbar, wenn W02 zuerst läuft: dessen Reparaturen sitzen im Ladepfad, T3-02-06 im
  Schreib- und Rollenpfad.
- **W05 ↔ keiner der sechs.** T3-01-02, T3-01-03 und T3-01-05 betreffen Sperren am
  Parameter- und Offlineeingang und den 5-ms-Wandzeittakt. Keine der hier gelesenen Ketten
  läuft darüber. Die einzige mögliche Berührung ist die Sperrenordnung: der Fix zu
  T3-04-01 fügt **keine** neue Sperre hinzu (die Schleuse arbeitet im Callbackpfad nur mit
  Atomics, `Schleuse.h:28-37`), der Fix zu T3-04-03 ebenfalls nicht. Falls W05 die
  Sperrenordnung verschiebt, bleiben beide davon unberührt.

---

## 10. Reihenfolge

Ich schlage **vier Etappen** vor. Der Maßstab ist die Regel der Arbeitsübersicht
(`BEFUNDE.md:796`): Audiofehler, Zustandsverlust und hängende Rückwege vor
Dokumentationshärtungen — und davor noch das, was ohne Antwort nicht wartet.

**Etappe 1 — Besitz (W09, kein Produktentscheid).**
T3-04-01 und T3-04-03. Grund für den ersten Platz: es sind die einzigen zwei der sechs,
deren Folge undefiniertes Verhalten ist, und beide brauchen keine Antwort. T3-04-01 zieht
die Testzugänge der Sonde nach (Testkonstruktor, Starthaken, Probe-Pipename) — eine
Bauvoraussetzung, von der auch spätere Sonde-Lebenslaufbeweise leben. T3-04-03 ist die
kleinere Hälfte und kann im selben Ticket laufen, weil die Prüfform dieselbe ist (Wurzel 1).

**Etappe 2 — Marker und Fläche (T3-07-02, kein Produktentscheid).**
Der Befund verletzt eine datierte Userentscheidung, betrifft das gehörte Signal und hat
einen kleinen, dreiteiligen Fix. **Gemeinsam mit W06/T3-01-10 planen** (Gen-Reset bei
aktiver Hörmarkierung) — beide bewegen dieselben Rücknahmewege, und zwei getrennte Tickets
würden einander überschreiben. Wenn W06 aus Teil A zuerst dran ist, gehört T3-07-02 in
dessen Ticket; sonst umgekehrt.

**Etappe 3 — Quellen und Text (T3-14-01 ganz; T3-07-05 zur Hälfte).**
T3-14-01 braucht keine Antwort und verletzt dieselbe Entscheidzeile wie Etappe 2. Von
T3-07-05 ist der Teil ohne Antwort baubar, der heute den größeren Schaden trägt: das
Ersatzziel folgt der Anzeigeordnung, sodass kein sichtbarer Knopf auf eine ungezeichnete
Quelle zeigt (6 g), letzter Absatz). Beide sitzen in
`aktualisiereSourcesSteuerung` (`PluginEditor.cpp:902-944`) — ein Änderungssatz, eine
Prüfung.

**Etappe 4 — nach den Antworten.**
- **T3-02-06** wartet auf **U49**. Vorher baubar: der Rotbeweis aus 3 h), der das heutige
  Verhalten in beide Richtungen festhält, und die Berichtigung des Kartentexts. Nicht
  vorher baubar: die Regel selbst, weil die zwei Wege gegenläufig sind (Weg 1 legt Daten
  an, Weg 2 löscht sie). **Nach W02 einplanen**, weil beide den Ladepfad berühren.
- **Die Kapazitätsgrenze aus T3-07-05** wartet auf **U51**. Vorher baubar: die Shots mit
  20, 21, 32 und 64 Quellen, die die heutige Grenze messen, und die Gegenwache in B14
  (State-Bestand bleibt unverändert).

**Was an keiner Antwort hängt und sofort gemacht werden kann**, unabhängig von der
Ticketreihenfolge: die zwei irreführenden Kommentare berichtigen —
`PluginEditor.cpp:731-733` (behaupteter SafePointer-Schutz für den Prozessorzugriff) und
`PluginEditor.cpp:268-269` (drei Sicherheitsnetze, die in der Main-Fläche nicht ticken).
Beide sind reiner Text und machen die nächste Lesung aller Beteiligten ehrlicher. Sie
gehören trotzdem in das Ticket, das die Stelle repariert, nicht in ein eigenes.

---

## 11. Nicht geprüft

- **Keine Messung, kein Lauf.** Der Auftrag verbot Bau, Testlauf, Kanon, Skript, FL Studio
  und Fehlerinjektion. Alles oben ist Quellenlektüre. Die Zahlenwerte aus Phase 16
  (Ressourcen, Zyklen) habe ich zitiert, nicht nachgemessen. Die einzige Rechnung, die ich
  selbst ausgeführt habe, ist die Zeilenarithmetik in Abschnitt 6 b
  (366 px, `jlimit(18,34,·)`, 20 Zeilen); sie ist Ganzzahlarithmetik aus vier gelesenen
  Zeilen und braucht keinen Lauf.
- **Kein Hostlauf.** Ob FL die Reihenfolge Editorabbau → Prozessorabbau → modale
  Aufräumnachricht tatsächlich so fährt, wie T3-04-03 sie braucht, ist **nicht belegt**.
  Ich habe gezeigt, dass JUCE sie zulässt (`juce_ModalComponentManager.cpp:81-90`, `:196-212`;
  `juce_audio_plugin_client_VST3.cpp:2326-2327`), nicht, dass FL sie wählt. Ebenso
  unbelegt: ob ein Provider der Sonde die 2-Sekunden-Frist in der Praxis je reißt.
- **Der lokale JUCE-Stand ist nicht gegen den Bau verifiziert.** Ich habe
  `eq-copilot/build/_deps/juce-src/` gelesen — ein Bauartefakt. Phase 14 nennt den
  gepinnten Stand `f72bad64d29715216226685810c5196bd0d79d77`
  (`roh/phase-14-blindspots.md:41`); dass der Baum im Arbeitsverzeichnis genau dieser
  Stand ist, habe ich **nicht** geprüft (das hätte einen Git-Befehl im Unterbaum gebraucht).
  Die zitierten JUCE-Zeilen sind deshalb als „der Stand, der hier liegt" zu lesen.
- **Die Fokusfolge von T3-14-01 ist hergeleitet, nicht gefahren.** Dass ein Klick auf ein
  gezeichnetes Rechteck dem `TextEditor` den Tastaturfokus nicht nimmt, stützt sich auf die
  Bauart (`sourcesZeilen()` liefert Rechtecke, keine Komponenten) und auf die
  JUCE-Fokusanalyse von Phase 14 (`roh/phase-14-blindspots.md:41`). Ich habe die drei dort
  genannten JUCE-Zeilen **nicht** einzeln nachgelesen.
- **Die anderen acht IDs von NAK-312 sind nicht Gegenstand.** W02, W05 und W06 gehören dem
  Parallelagenten (Teil A); meine Aussagen dazu in Abschnitt 9 sind Hinweise aus den
  Registerzeilen, keine Quellvalidierung.
- **Sicherheitsbefunde T3-12-01 bis T3-12-05** sind ausgeschlossen und wurden nicht gelesen.
- **`docs/plan/fragen.json` ist nicht geändert.** Die Textvorschläge zu U49 und U51 stehen
  nur hier.
- **Die Standardgröße 950×538** aus `CLAUDE.md` habe ich als Rechengrundlage für die
  künftige Zeilenzahl benutzt (26 Zeilen), aber **nicht** als Befund behandelt: die
  Abweichung des Codes (760×430, `PluginEditor.cpp:858`) gehört zum geparkten Designteil.
  Falls der Dirigent sie als Widerspruch zwischen Entwurf und Code führen will, gehört
  dafür eine Zeile nach `docs/offene-punkte.md` — das ist außerhalb dieses Auftrags.
- **Kein Dokuriegel gelaufen.** `tools/plan/dokuriegel.py` ist ein Skript; der Auftrag
  verbietet Skripte. Diese Datei ist ein Rohbeleg unter `docs/beweise/roh/`, kein
  Plandokument.

---

FERTIG Teil B, 4 bestätigt, 2 präzisiert, 0 widerlegt.
