# NAK-309 — Quellvalidierung Teil B (T3-13-01 bis T3-13-06)

**Ticket:** NAK-309, Planschritt S25h, Etappe 0 „Quellvalidierung", **Teil B**.
**Datum:** 18.09.2026. **Modell:** Opus, lesender Validierungsagent.
**IDs:** T3-13-01, T3-13-02, T3-13-03, T3-13-04, T3-13-05, T3-13-06 (Phase 13 des
Tiefenaudits vom 15.09.2026, `docs/audits/2026-09-15-tiefenaudit/BEFUNDE.md:566-571`,
Herleitungen in `docs/audits/2026-09-15-tiefenaudit/roh/phase-13-bauhostkette.md:27-80`).
Phase 13 hatte keinen Skeptikerpass; dies ist die erste Gegenprüfung.

**HEAD Nakama vor der Arbeit:** `4c1c7f3c8cb5145de0e545aebe6305e450f00f7c`.
**HEAD Nakama nach der Arbeit:** `3b4e8bd0d46fbf10aaeb8a64d7e9a4878cd77a6a` — eine
parallele Session hat während meines Laufs `3b4e8bd0` („Karten U44 bis U52") committet.
`git diff --stat 4c1c7f3c 3b4e8bd0 -- eq-copilot/ broker/ tools/ CLAUDE.md docs/plan/plan.json
docs/beweise/NAK-286.md docs/gesundheit/ docs/audits/ docs/offene-punkte.md
docs/plugin-wissen.md` ist **leer**; geändert hat sich nur `docs/plan/fragen.json`. Jede
Zeilennummer und jedes Zitat unten gilt an beiden Ständen unverändert.

**HEAD fl-studio-mcp vor und nach der Arbeit:** `01f72386bfc7eab507a40b9c96ef9e2e2a0265cc`
(Zweig `evenacadia-local`), `git status --short` dort **leer** vor und nach dem Lauf. Das
Nachbarrepo wurde ausschließlich gelesen.

**`git status --short` vor dem Schreiben dieser Datei:**

```text
 M docs/PLAN-STAND.md
?? briefing-hub/
?? docs/beweise/NAK-309.md
?? nimbalyst-local/
```

Damit **nicht** wie erwartet: neben den zwei fremden Ordnern `briefing-hub/` und
`nimbalyst-local/` sind **zwei weitere fremde Einträge** dazugekommen — `docs/PLAN-STAND.md`
(geändert) und `docs/beweise/NAK-309.md` (neu, untracked). Beide stammen aus der parallelen
Dirigentensession, nicht von mir. Ich habe keinen der vier Einträge angefasst; meine einzige
Schreibaktion ist diese Datei.

**Prüfform:** ausschließlich Quellenlektüre und lesendes Git. Kein Bau, kein Test, kein
Kanon, kein FL-Start, kein `uv run`, kein MCP-Aufruf, kein Codex. Sicherheitsbefunde
(T3-12-*) sind ausgeschlossen; Probe-Pipe gegen Produktions-Pipe wird hier als
Korrektheitsfrage nach `CLAUDE.md:195` behandelt, nicht als Sicherheitsprüfung.

**Erreichbarkeitsstufen:** 1 = heutiger Produkt- oder Werkzeugeingang, 2 = gebaute API ohne
Aufruf, 3 = erst nach anderer Reparatur, 4 = noch nicht gebaut.

---

## T3-13-01 — PipeProbe ohne Pipeargument nutzt die Produktions-Pipe

*Audit: high · K3 · DEFEKT.*

### 1. Quellkette, zitiert

`eq-copilot/plugin/probe/PipeProbeMain.cpp:109-117` — beide Vorgabewerte in einem Block:

```cpp
int main (int argc, char** argv)
try
{
    const juce::String pipeName = argc > 1 ? juce::String (argv[1])
                                           : juce::String (juce::CharPointer_UTF16 (eqcop::kPipeName));
    const int sekunden = argc > 2 ? juce::jlimit (1, 60, juce::String (argv[2]).getIntValue()) : 5;
    const juce::String serverBinary = argc > 3 ? juce::String (argv[3]) : juce::String();

    const auto erwartung = serverErwartungFuer (serverBinary);
```

`eq-copilot/plugin/src/EqCopilotIds.h:16-18` — der Vorgabename ist die Produktions-Pipe:

```cpp
// Fester Name; Zugriff wird broker-seitig per ACL auf den aktuellen Windows-User
// begrenzt (Plan §9.2). Das Session-Token kommt aus dem Welcome des Brokers.
inline constexpr const wchar_t* kPipeName       = L"\\\\.\\pipe\\evenacadia.eq-copilot.v1";
```

`eq-copilot/plugin/src/PipeClient.cpp:216-223` — **zweiter** Rückfall auf denselben Namen,
falls ein leeres erstes Argument durchgereicht wird:

```cpp
PipeClient::PipeClient (std::function<HelloInfo()> hp,
                        std::function<StatsSnapshot()> sp,
                        std::function<MessKompakt()> mp,
                        const juce::String& name,
                        std::chrono::milliseconds timeout,
                        nakama::ipc::ServerErwartung serverErwartungIn)
    : helloProvider (std::move (hp)),
      ...
      pipeName (name.isNotEmpty() ? name : juce::String (juce::CharPointer_UTF16 (kPipeName))),
```

`eq-copilot/plugin/probe/PipeProbeMain.cpp:53-59` — ohne drittes Argument ist der
**installierte Produktionsbroker** die Erwartung, die Authentisierung ist also erfüllt und
verhindert diesen Fall nicht:

```cpp
static nakama::ipc::ServerErwartung serverErwartungFuer (const juce::String& serverBinary)
{
    if (serverBinary.isEmpty())
        return nakama::ipc::ServerErwartung {
            nakama::ipc::installbindung::brokerPfad,
            nakama::ipc::installbindung::brokerSha256,
            nakama::ipc::installbindung::authenticodeThumbprint };
```

Was auf dieser Pipe gemeldet wird, steht in `PipeProbeMain.cpp:31-45` und `:72-78` — eine
**Rolle `sensor` mit frei erfundenen Messwerten**:

```cpp
    m.metricsVersion = "m3-2026-08-14";   // aktueller kMetricsVersion-Stand
    m.aktivS = 12.0;
    m.gesamtS = 14.0;
    m.lufsGueltig = true;  m.lufsI = -18.5;
    m.truePeakGueltig = true; m.truePeakDb = -3.2;
    m.ltasKompositDb.assign (221, -60.0);
```

```cpp
    hello.sensorId = sensorId;
    hello.instanceNonce = juce::Uuid().toString();
    hello.role = "sensor";
```

**Auslöser und Erreichbarkeit — hier weicht die Prüfung vom Audit ab.** Die Antwort auf die
Auftragsfrage „welches Bein in `tools/beweise.ps1` ruft PipeProbe wie auf?" lautet: **keines.**

- `git grep -In "PipeProbe\|pipe-probe" -- tools/` liefert **null Treffer** (an HEAD
  `4c1c7f3c` und `3b4e8bd0`).
- Die Bauliste des Runners entsteht aus den Beinnamen selbst,
  `tools/beweise.ps1:883-886`:

```powershell
    $zuBauen = @($kanon | Where-Object { $_.Art -eq 'plugin' -and -not (Ist-Stillgelegt $_) -and $cmakeText -match [regex]::Escape($_.Name) } | ForEach-Object { $_.Name })
    $zuBauen += @($gemesseneZiele | Where-Object { $cmakeText -match [regex]::Escape($_.Marker) } | ForEach-Object { $_.Ziel })
```

  `EqCopPipeProbe` ist weder Beinname noch Marker eines gemessenen Ziels — der Kanon **baut
  das Werkzeug nicht einmal**.
- Das ist im Repo bereits festgehalten, `docs/beweise/NAK-289.md:514-519` (NB-1): „`eqcop-pipe-probe.exe`
  (`probe/PipeProbeMain.cpp`) steht weder in der Beinliste noch in der Bauliste von
  `tools/beweise.ps1`; `git grep` über `tools/` findet keinen Aufrufer."

Auslöser ist daher **allein ein Handaufruf**: ein Mensch baut das Ziel eigens
(`cmake --build eq-copilot/build --config Release --target EqCopPipeProbe`) und startet es
**ohne erstes Argument**. **Erreichbarkeit: Stufe 1**, aber ein Werkzeugeingang ohne
automatischen Aufrufer. Beide dokumentierten Abläufe nennen ausdrücklich einen Probe-Namen
(`eq-copilot/README.md:55-62`, `docs/plugin-wissen.md:881-886`); die fehlerhafte Form ist die
**kürzeste**, nicht die dokumentierte.

### 2. Zusage, wörtlich

`CLAUDE.md:195` (tragende Invariante):

> **Begriffe nicht vermischen:** Probe-Pipe ist nie Produktions-Pipe;

`CLAUDE.md:227-228` (Abschnitt „Bauen und beweisen"):

> Broker-Ende-zu-Ende immer über den Probe-Pipenamen; nie ein Testbroker auf
> der Produktions-Pipe.

`eq-copilot/README.md:55` trägt dieselbe Zusage in der Bedienanleitung des Werkzeugs:

> Pipe Ende-zu-Ende (immer der eigene Probe-Pipename, nie die Produktion):

Die Zusage ist damit belegt und dreifach. **Präzisierung gegenüber dem Audit:**
`CLAUDE.md:227` spricht wörtlich vom *Testbroker*, nicht vom Probe-*Client*; die Zeile, die
den hier verletzten Fall exakt trifft, ist `CLAUDE.md:195` plus `README.md:55`. Das Gate
S25e (`docs/plan/plan.json`, GRENZE: „keine Produktions-Pipe für Testbroker") deckt diesen
Fall ebenfalls nur über die Brokerseite ab.

### 3. Heutige Abdeckung

- **Kein Bein, kein Riegel, kein Test.** Die einzigen Prüfungen, die `PipeProbeMain.cpp`
  überhaupt berühren, sind zwei **Quelltextwachen** aus NAK-289 in
  `eq-copilot/plugin/tests/PipeClientLifecycleTestMain.cpp:1408-1418` (Bein **A4b**,
  `tools/beweise.ps1:505-511`). Ihr Kopfkommentar sagt selbst, warum:

```cpp
// NAK-289 Etappe 1: das Probewerkzeug eqcop-pipe-probe.exe
// (probe/PipeProbeMain.cpp) faehrt kein Bein. Diese Wachen lesen deshalb
// seinen QUELLTEXT und halten die Zusagen fest, die NAK-289 dort eingefuehrt
// hat. Der Pfad kommt aus __FILE__, wie in DspGoldenTestMain.cpp.
```

  Geprüft werden dort nur Ausnahmegrenze und Exit 70 (`:1428-1452`) sowie Zeilenende mit
  Flush — **nie der Pipename**.
- Die vorhandene Erlaubnisliste `nakama::ipc::istProbePipename`
  (`eq-copilot/plugin/core/ipc/PipeToken.h:59-70`) **deckt diesen Pfad nicht ab und kann
  ihn nicht ohne Weiteres abdecken**:

```cpp
inline constexpr const char* kPipePraefixProbe =
    "\\\\.\\pipe\\evenacadia.nakama.v3.probe.";
```

  Sie erlaubt ausschließlich den **v3**-Probenamensraum. Der dokumentierte v2-Ablauf des
  Werkzeugs benutzt `\\.\pipe\evenacadia.eq-copilot.m2probe…`
  (`eq-copilot/README.md:61`, `docs/beweise/roh/NAK-123-r1-d8676e0.md:623`), der von
  `istProbePipename` **abgewiesen** würde. Ein naiver Fix „nimm die vorhandene Allowlist"
  macht das Werkzeug funktionsunfähig — genau die Falle, in die NAK-123 P2 schon einmal
  gelaufen ist (`docs/beweise/NAK-123.md:482`).
- Registerdeckung: keine. Kein Eintrag in `docs/offene-punkte.md` erlaubt diesen Vorgabewert.

### 4. Urteil

**BESTÄTIGT, präzisiert.** Die Aufrufkette ist an der Quelle genau so, wie das Audit sie
beschreibt; der Sicherheitsnachweis gegen den installierten Broker greift nicht, weil der
echte Produktionsbroker der erwartete Gegenprozess ist. Drei Präzisierungen:

1. **Kein Bein und kein Runner ruft oder baut PipeProbe** (`git grep` über `tools/` = 0;
   Bauliste `beweise.ps1:883-884`; belegt in `NAK-289.md:514`). Die Arbeitsübersicht V1
   („Stufe 1 im Beweiswerkzeug") gilt nur im engen Sinn eines Handaufrufs.
2. **Zwei Rückfallstellen, nicht eine:** `PipeProbeMain.cpp:112-113` (fehlendes Argument)
   **und** `PipeClient.cpp:221` (leerer Name). Ein Fix nur an der ersten Stelle lässt
   `eqcop-pipe-probe.exe ""` weiterhin auf der Produktions-Pipe landen.
3. **Der konkrete Schaden ist benennbar:** nicht ein Konflikt mit einer echten Instanz — die
   Sensor-ID ist eine frische UUID (`:131`) und kollidiert nicht — sondern eine
   **Phantomquelle mit erfundenen Messwerten** (`probeMessung()`, `role = "sensor"`) im
   Aggregat des laufenden Produktionsbrokers, für die Dauer des Laufs (Vorgabe 5 s, bis 60 s).

**Kategorievorschlag: DEFEKT, Schwere medium** (Herabstufung von high). Begründung: kein
automatischer Aufrufer, das Ziel wird vom Kanon nicht einmal gebaut, und beide dokumentierten
Abläufe nennen einen Probe-Namen. Gegenargument, das der Dirigent höher gewichten kann: die
verletzte Regel ist eine ausdrücklich benannte tragende Invariante, und die fehlerhafte
Aufrufform ist die bequemste. Der Dirigent entscheidet.

### 5. Kleinster Fix und Ort des Rotbeweises

Die Namensentscheidung aus `main` in eine reine, prüfbare Funktion ziehen — neuer Kopf
`eq-copilot/plugin/src/ProbePipeRegel.h` (das Verzeichnis `src` liegt bereits auf dem
Include-Pfad von `EqCopPipeProbe`, `CMakeLists.txt:604`, **und** von `EqCopPipeClientTest`,
`CMakeLists.txt:619`) mit einer Funktion, die einen leeren Namen und `eqcop::kPipeName`
abweist und einen v2-Probenamen sowie den v3-Probepräfix zulässt; `PipeProbeMain.cpp:112-117`
ruft sie vor `baueClient` und endet bei Ablehnung mit einer eigenen Meldung und Exit 2. Der
Vorgabewert entfällt ersatzlos: ohne erstes Argument endet das Werkzeug mit der Nennung der
erlaubten Form.

**Rotbeweis: Bein A4b (`EqCopPipeClientTest`, `tools/beweise.ps1:505-511`), an der neuen
Funktion, nicht an einem Nebeneffekt.** Drei Fälle im vorhandenen Testmain: leerer Name →
abgewiesen; `eqcop::kPipeName` → abgewiesen; `\\.\pipe\evenacadia.eq-copilot.m2probe.rot` →
zugelassen. Vor dem Fix ist der Fall rot, weil die Funktion nicht existiert und der bestehende
Pfad den Produktionsnamen annimmt (Gegenprobe: die Funktion einmal mit `return true` bauen →
Fälle 1 und 2 rot). Nach dem Fix grün. Der dritte Fall ist der Riegel gegen die NAK-123-Falle:
wer `istProbePipename` ohne v2-Zweig einsetzt, macht ihn rot.

---

## T3-13-02 — Laufzeit-Arm beendet ein fremdes FL per Titel-Teilstring

*Audit: critical · K2 · DEFEKT.*

### 1. Quellkette, zitiert

`tools/fl/laufzeit.ps1:95` — der Vergleichsschlüssel ist ein reiner Dateiname:

```powershell
        Projekt = $projekt; ProjektName = [IO.Path]::GetFileName($projekt); Sample = $w.Sample
```

`tools/fl/laufzeit.ps1:338-351` — Instanzliste und die beiden Klassifikatoren:

```powershell
function FL-Instanzen {
    # NAK-297: der Titel eines FL-Prozesses kommt aus seinem Hauptfenster der Klasse
    # TFruityLoopsMainForm; MainWindowTitle liefert unter FL 2026 manchmal das leere
    # TApplication-Fenster. Ein leerer Titel ist kein Fremdprojekt.
    foreach ($p in @(& $script:U.Prozesse)) {
        $haupt = @(& $script:U.Fenster $p.Id | Where-Object { $_.Klasse -eq 'TFruityLoopsMainForm' -and $_.Sichtbar })
        ...
}
function Ist-Diagnose($inst) { $inst.Fenster -and $inst.Titel -like "*$($script:K.ProjektName)*" }
function Ist-Fremd($inst)    { $inst.Fenster -and $inst.Titel -and -not ($inst.Titel -like "*$($script:K.ProjektName)*") }
```

`tools/fl/laufzeit.ps1:353-360` — das Beenden, ohne jede weitere Bedingung:

```powershell
function Beende-Diagnose-FL {
    foreach ($i in @(FL-Instanzen | Where-Object { Ist-Diagnose $_ })) {
        Log "Diagnose-FL beenden: PID $($i.Id) '$($i.Titel)' (Projekt wird nie gespeichert)"
        & $script:U.BeendeProzess $i.Id
    }
```

`tools/fl/laufzeit.ps1:250` — der Adapter ist der echte, erzwingende:

```powershell
    $u.BeendeProzess = { param([int]$id) Stop-Process -Id $id -Force -ErrorAction SilentlyContinue }
```

Gemessene Titelform aus einem echten Lauf,
`docs/beweise/roh/NAK-286-laufzeit-34bdf159-lauf1.md:49`:

```text
[2026-09-15 03:17:58] Ping ok: FL Producer Edition v26.1.4 [build 5589] ('FL Studio 2026'), Fenster 'Nakama-Diagnose.flp - FL Studio 2026', Port loopMIDI Port 1, Diagnose-PID 25560
```

**Auslöser — sieben Stellen, jede mit vorgeschalteter, aber blinder Fremdprüfung.** Der
Runner **fragt nirgends** (er ist ausdrücklich für den unbeaufsichtigten Betrieb gebaut) und
er **prüft überall** — die Prüfung ist nur die logische Negation desselben Teilstringtests:

| Stelle | Zeilen | Vorgeschaltete Prüfung |
| --- | --- | --- |
| `Installiere` | :403-406 | `Ist-Fremd` → Exit 0 UEBERSPRUNGEN |
| `Pruefe-Controller` | :443-445 | `Ist-Fremd` → Exit 0 UEBERSPRUNGEN |
| `Rendere` | :564-567 | `Ist-Fremd` → Exit 0 UEBERSPRUNGEN |
| `Frischer-Start` | :697-701 | `Ist-Fremd` → Exit 0 UEBERSPRUNGEN |
| `Fahre-Rueckweg` | :712-716 | keine (nach ABWEICHUNG) |
| `Fahre-Lauf`, Neustart | :807-809 | `Ist-Fremd` nur, wenn `$laufend.Count -eq 0` (:808) |
| `Fahre-Lauf`, `-Beenden` | :820 | keine |

Die Schranke bei `:808` ist der Kern des Befunds:

```powershell
            if ($fremd.Count -gt 0 -and $laufend.Count -eq 0) { Ende 0 'UEBERSPRUNGEN' "fremdes FL-Projekt offen: '$($fremd[0].Titel)'" }
            if ($K.Neustart -and $laufend.Count -gt 0) { Beende-Diagnose-FL; $laufend = @() }
```

Ein fremder Prozess mit passendem Titel ist **nicht** in `$fremd` (weil `Ist-Fremd` falsch
ist) und **ist** in `$laufend` — beide Bedingungen fallen zu seinen Ungunsten aus.

**Bedingungen, die ein fremdes Projekt erfüllen muss, um beendet zu werden** (ausdrücklich
verlangte Antwort):

1. Prozessname `FL64`, `FL` oder `FL Studio` (`:240-243`).
2. Dieselbe Windows-Sitzung, so dass `Get-Process` ihn sieht und `Stop-Process -Force`
   greift; Zugriffsverweigerung wird still geschluckt (`-ErrorAction SilentlyContinue`).
3. Ein sichtbares Hauptfenster der Klasse `TFruityLoopsMainForm` (oder ersatzweise ein
   `MainWindowHandle` ungleich 0, `:347`), dessen Titel die Zeichenkette
   `Nakama-Diagnose.flp` **enthält**. Das trifft nicht nur eine Datei gleichen Namens in
   einem anderen Ordner, sondern jeden Dateinamen, der die Kette enthält, etwa
   `Kopie Nakama-Diagnose.flp`.
4. Der Lauf erreicht eine der sieben Stellen oben.

**Erreichbarkeit: Stufe 1, und der Weg dorthin ist im Projekt selbst vorgezeichnet.** Der
Handgriff **K-286-2** verlangt vom User, genau dieses Projekt von Hand zu öffnen,
`tools/fl/LIES-MICH.md:122-126`:

> Nach `mixer.focusEditor` erfasste `lokal.fenster` das Fenster von Nakama
> Probeeq auf Insert 1 im Diagnoseprojekt nur als Kopfleiste des eingeklappten
> FL-Wrappers (`TPluginForm`, 67 × 31 Pixel, 15.09.2026). Aufklappen und
> Speichern ist der Handgriff K-286-2 (Karte U43)

Das Diagnoseprojekt von Hand geöffnet und ungespeichert stehen gelassen, dazu ein
nächtlicher Volllauf (`-Erzwingen -Beenden`, Matrixzeile M-13,
`docs/beweise/NAK-286.md:520`) — und `:820` beendet die Handarbeit des Users erzwungen.
Dieser konkrete Weg steht nicht in der Herleitung des Audits; er macht den Befund **stärker**,
nicht schwächer.

### 2. Zusage, wörtlich

Gate S25e, `docs/plan/plan.json` (Schlusssatz des Feldes `text`, Schritt-ID `S25e`):

> GRENZE: keine Legacy-Umbenennung (NAK-30), keine Produktions-Pipe für Testbroker, kein
> Speichern des Diagnoseprojekts aus der Automatik, **kein Beenden eines FL mit fremdem
> Projekt.**

Dazu, nicht vom Audit genannt, die **Behauptungszeile des Kanonbeins A35**,
`tools/beweise.ps1:618`:

> Der Selbsttest des Laufzeit-Arms (Plan S25e, NAK-286) laeuft ohne FL, ohne Installation
> und ohne MCP-Repo gegen Attrappen: der Runner haelt Exitcodes, Urteilswoerter und
> Kopfzeile der ersten Fassung, **beendet nie ein fremdes FL**, …

Ein Bein behauptet also heute im Kanon etwas, das es für diesen Fall nicht misst.

### 3. Heutige Abdeckung

- **Selbsttest `fremdes_projekt_nie_beenden`** (`tools/fl/laufzeit.ps1:1093-1117`, Bein A35).
  Er deckt drei Stellen ab — vor der Installation, vor dem Render, vor dem FL-Start — aber
  ausschließlich mit einem Titel **ohne** den Diagnosenamen:

```powershell
Fall 'M-03' 'fremdes_projekt_nie_beenden' {
    Testfall 'fremdes FL vor der Installation'
    $fremd = T-Prozess 'Mein Song.flp - FL Studio 2026'
```

  Der Kollisionsfall ist damit **nicht abgedeckt**.
- **Matrixzeile M-03**, `docs/beweise/NAK-286.md:510`, deckt das heutige Verhalten sogar
  ausdrücklich, weil sie „fremd" selbst über denselben Teilstring definiert:

> **FL:** fremdes FL — ein FL64-Prozess mit Fenster, dessen Titel `Nakama-Diagnose.flp`
> nicht enthält … der fremde Prozess wird nie beendet (`Stop-Process` nur für
> Diagnose-Instanzen und fensterlose Prozesse, `:112-131`)

  Das ist die wichtigste Präzisierung dieses Abschnitts: **die abgenommene Matrix
  legitimiert den Code.** Eine Reparatur, die M-03 stehen lässt, erzeugt einen Widerspruch
  zwischen Matrix und Gate.
- **Registerzeile NAK-297** (`docs/offene-punkte.md:513`, geschlossen 15.09.2026) hat den
  benachbarten Fall behandelt — leerer `MainWindowTitle` — und behauptet im Schlusssatz
  ausdrücklich: „ein FL mit fremdem Projekt ist davon nicht betroffen (R-286-5 hält)". Das
  gilt für den dort gemessenen Fall und trägt den hier gefundenen nicht.
- **Registerzeile NAK-290** (`docs/offene-punkte.md:300`) betrifft parallele Kanons auf
  geteilten Probe-Pipenamen und hat mit diesem Befund nichts zu tun.

### 4. Urteil

**BESTÄTIGT und verschärft.** Der Code tut genau, was das Audit beschreibt; die Erreichbarkeit
ist über den projekteigenen Handgriff K-286-2 höher als dargestellt. Drei Präzisierungen:

1. **Der Runner fragt nie und prüft überall — die Prüfung ist blind für genau diesen Fall**,
   weil `Ist-Fremd` (`:351`) die exakte Negation von `Ist-Diagnose` (`:350`) ist.
2. **M-03 muss im selben Änderungssatz mit korrigiert werden.** Sonst steht eine abgenommene
   Matrixzeile gegen das Gate.
3. **Zweiter Pfad, bewusst so gebaut, ausdrücklich nicht Teil dieses Fixes:**
   `Beende-Restprozesse` (`:361-369`) beendet jeden FL-Prozess **ohne** Fenster ohne jede
   Fremdprüfung. M-03 (`NAK-286.md:510`) und M-07 (`:514`) erlauben das ausdrücklich. Ein
   fremdes FL, das sein Hauptfenster noch nicht erzeugt hat, ist dort nicht unterscheidbar.
   Ich habe das **nicht gemessen** (kein FL-Start erlaubt) und leite es nur aus `:347` ab;
   ob die Grenze mitgezogen wird, ist eine Entscheidung des Dirigenten, keine Folge dieses
   Befunds.

**Kategorievorschlag: DEFEKT, Schwere critical (bestätigt).** Der unmittelbare Schaden ist
Datenverlust an der Arbeit des Users durch ein unbeaufsichtigtes Werkzeug.

### 5. Kleinster Fix und Ort des Rotbeweises

Das Beenden an **Besitz** binden statt an den Titel: `Starte-FL` (`:642-648`) setzt bereits
`$script:K.DiagnosePid`; `Beende-Diagnose-FL` beendet nur Instanzen, deren PID der Lauf selbst
gestartet hat, und `Warte-Ping` (`:679-683`) übernimmt eine vorgefundene PID nur, wenn sie
mit der gestarteten übereinstimmt. Eine vorgefundene Instanz mit passendem Titel, die der
Lauf nicht gestartet hat, ist ab dann **Szenario-Voraussetzung**: Exit 0 `UEBERSPRUNGEN` mit
Titel und PID im Grund, genau wie heute schon ein fremdes Projekt. M-03 im selben Satz
umformulieren: „fremd" heißt „nicht von diesem Lauf gestartet", nicht „Titel ohne
Diagnosenamen".

**Rotbeweis: Attrappenfall in `laufzeit.ps1 -Selbsttest` (Bein A35), an der Zusagezeile des
Gates, nicht an einem Nebeneffekt.** Neuer Fall neben `fremdes_projekt_nie_beenden`, mit dem
vorhandenen Attrappenbauer `T-Prozess` (`:907-918`), der eine Instanz erzeugt, **ohne** dass
der Lauf sie gestartet hat:

```powershell
Fall 'M-03' 'namensgleiches_fremdes_projekt_nie_beenden' {
    Testfall 'fremdes FL mit demselben Projektdateinamen vor der Installation'
    $fremd = T-Prozess 'Nakama-Diagnose.flp - FL Studio 2026'
    $e = T-Lauf
    Pruefe (-not (T-Hat "beende $fremd")) 'fremdes namensgleiches FL beendet'
    Pruefe ($e.Code -eq 0 -and $e.Urteil -eq 'UEBERSPRUNGEN') "Exit $($e.Code) $($e.Urteil)"
}
```

Vor dem Fix **rot**: `Installiere` (`:405`) erreicht `Beende-Diagnose-FL` und die Attrappe
protokolliert „beende <pid>". Nach dem Fix grün. Ein zweiter Fall deckt `-Beenden`
(`:820`) mit derselben Attrappe ab, ein dritter den Neustartpfad (`:809`) mit gesetztem
`-Neustart`. Alle drei brauchen **keinen echten FL-Lauf**; die Fassade `Neue-Testumgebung`
fängt `Stop-Process` ab.

---

## T3-13-03 — Verspätete MCP-Antwort wird dem nächsten Befehl zugeordnet

*Audit: high · K1 · LÜCKE. Bekannt NAK-307; geprüft wird nur das Darüberhinausgehende.*

### 1. Quellkette, zitiert

Client, `fl-studio-mcp` (HEAD `01f72386`)
`src/fl_studio_mcp/utils/midi_connection.py:214-242` — Auftrag ohne Kennung, gemeinsame
Dateien, Trigger:

```python
        # Prepare command
        command = {
            "action": action,
            "params": params or {},
        }

        # Write command to file
        try:
            self._command_file.write_text(json.dumps(command, indent=2))
        except Exception as e:
            return {"success": False, "error": f"Failed to write command file: {e}"}

        # Clear old response file
        if self._response_file.exists():
            try:
                self._response_file.unlink()
```

`:256-268` — jede parsebare Antwort gilt als **die** erwartete:

```python
        while time.time() - start_time < timeout:
            if self._response_file.exists():
                try:
                    response_text = self._response_file.read_text()
                    response = json.loads(response_text)
                    ...
                    return response
```

Controller, `fl_controller/device_FLStudioMCP.py:138-168` — Auftrag wird **zu Beginn**
gelesen, Antwort **am Ende** geschrieben, ohne Rückgabe einer Korrelation:

```python
        command_text = COMMAND_FILE.read_text()
        command = json.loads(command_text)

        action = command.get("action", "")
        params = command.get("params", {})

        # Execute command and get result
        result = dispatch_command(action, params)
        response = {"success": True, **result}
    ...
    write_response(response)
```

Nakama-Seite, `tools/fl/szenario.py:615-628` — der Eintrittspunkt des Befunds ist die
Wiederholung:

```python
def _sende(conn, aktion: str, params: dict | None, frist: float | None, umg: Umgebung) -> dict:
    """Ein Timeout wird genau einmal wiederholt, mit derselben Frist (M-78)."""
    ...
    antwort = einmal()
    if not antwort.get("success") and "Timeout" in str(antwort.get("error", "")):
        umg.schlafe(0.5)
        antwort = einmal()
        antwort["_wiederholt"] = True
    return antwort
```

`tools/fl/szenario.py:178-182` — die Bewertung prüft `success`, nie die Herkunft:

```python
def _pruefe(erwartung: dict, antwort: dict) -> tuple[list[str], list[str]]:
    """Verfehlte Erwartungen (leer = bestanden) und Rohwerte der Art `roh`."""
    fehler: list[str] = []
    roh: list[str] = []
    if not antwort.get("success"):
```

Das gemessene Beispiel, `docs/gesundheit/szenarien/bereitschaft.json:16-17` — zwei
aufeinanderfolgende Schritte mit **identischer** Erwartung:

```json
  {"aktion": "mixer.getPeaks", "params": {"track": 1}, "erwarte": {"peak_max": {"min": 0.01, "max": 1.0}}},
  {"aktion": "mixer.getPeaks", "params": {"track": 0}, "erwarte": {"peak_max": {"min": 0.01, "max": 1.0}}},
```

**Auslöser:** Szenario `bereitschaft.json` im Laufzeit-Arm, Schicht (4), gestartet über
`laufzeit.ps1:320-327` (`FahreSzenario` → `uv run --directory $McpRepo …`). Nötig sind
(a) ein Timeout in Schritt A, (b) eine zweite FL-seitige Ausführung, deren Lesen des
Auftrags vor und deren Schreiben der Antwort nach dem Beginn von Schritt B liegt.
**Erreichbarkeit: Stufe 1** — Timeouts sind belegt aufgetreten (NAK-307,
`docs/offene-punkte.md:315`).

### 2. Zusage, wörtlich

**Keine Zusage gefunden.** Die nächstliegenden Matrixzeilen sichern ausdrücklich etwas
anderes: M-20 (`docs/beweise/NAK-286.md:532`) sichert die **Fristwahl**, M-78 (`:533`)
sichert **genau eine** Wiederholung mit derselben Frist. Keine Zeile in
`docs/beweise/NAK-286.md`, im Gate S25e oder in `CLAUDE.md` sagt etwas über das **Eigentum**
einer Antwort.

**Fehlende Regel (Teil 6):** *Eine Antwort gilt nur dann als Ergebnis eines Schritts, wenn
sie nachweislich zu dessen Auftrag gehört; eine Wiederholung setzt die Zuordnung neu und
liegengebliebene Antworten früherer Aufträge werden vor dem nächsten Auftrag verworfen.*

### 3. Heutige Abdeckung

- `_sende` markiert eine Wiederholung im Rohprotokoll mit `"_wiederholt": true`
  (`tools/fl/szenario.py:627`), Selbsttest `timeout_genau_einmal`
  (`tools/fl/szenario.py:2398-2412`, Bein A35). Damit ist die **Eintrittsstelle** der
  Entkopplung im Beleg sichtbar — ein Mensch, der die Rohdatei liest, sieht, ab wo er
  misstrauisch sein muss. Die Zuordnung selbst wird nicht geprüft.
- Ein zweiter Timeout beendet den Schritt als verfehlt (M-78, `NAK-286.md:533`) — die
  Entkopplung wird also nicht beliebig lang fortgeschrieben, wohl aber über alle
  Folgeschritte, solange diese antworten.
- `_pruefe` (`:178-182`) verlangt `success`; nichts sonst.
- **Vom Audit nicht genannte, heute ungenutzte Deckung:** der Controller **gibt den
  Trackindex bereits zurück**, `fl_controller/device_FLStudioMCP.py:736-739`:

```python
    track = _check_track("track", params.get("track", 0), count)
    return {
        "track": track,
        "name": mixer.getTrackName(track),
```

  Für genau den Fall, den das Audit konstruiert, liegt das Korrelationsmerkmal also bereits
  in der Antwort und wird in `bereitschaft.json:16-17` nur nicht abgefragt.

### 4. Urteil

**BESTÄTIGT, präzisiert.** Die Ableitung hält an der Quelle: der Auftrag trägt keine Kennung
(`midi_connection.py:215-218`), die Antwort trägt keine (`device_FLStudioMCP.py:153, :166`),
der Leser nimmt jede (`:256-268`), und die Wiederholung (`szenario.py:623-627`) ist der
Eintritt. Drei Präzisierungen:

1. **Für den gemessenen Fall existiert eine Deckung zum Nulltarif**, die heute fehlt: die
   Antwort von `mixer.getPeaks` echot `track` und `name`. Die Behauptung „Request-ID,
   Aktions-, Ziel- und Instanzabgleich fehlen" ist für die **generische** Ebene richtig, für
   diesen konkreten Schritt aber zu stark.
2. **Die Zuordnung reißt nicht nur einmal:** nach Schritt B erbt Schritt C die Antwort von B
   und so weiter, bis ein Schritt keine Antwort mehr vorfindet und in ein Timeout läuft.
   Der Befund ist damit kein Einzelfehler, sondern eine **Kette**.
3. Der Ablauf ist **nicht im Host reproduziert** — das sagt das Audit selbst, und ich habe
   es nicht nachgeholt (kein FL-Lauf). Er ist aus den getrennten Dateioperationen zwingend
   herleitbar, das Zeitfenster ist die Dauer von `dispatch_command` (`:152`).

**Kategorievorschlag: LÜCKE, Schwere high (bestätigt).** Sie trifft K5 „Behauptung ≤ Messung"
im Kern: ein grüner Schritt kann eine Messung an einer anderen Spur sein.

### 5. Kleinster Fix und Ort des Rotbeweises

Zwei Stufen, die zweite ist die eigentliche Regel:

- **Sofort und im Nakama-Repo:** `docs/gesundheit/szenarien/bereitschaft.json:16-17` um die
  Echo-Erwartung ergänzen (`"track": {"gleich": 1}` bzw. `{"gleich": 0}`). Das macht genau
  den vom Audit konstruierten Ablauf sichtbar statt grün.
- **Regel:** `send_command` (`midi_connection.py:190-242`) hängt dem Auftrag eine
  unverwechselbare Kennung an, der Controller (`:153`) gibt sie in der Antwort zurück, und
  `_wait_for_response` (`:244-283`) verwirft jede Antwort mit fremder oder fehlender
  Kennung, statt zurückzukehren. Beides sind Änderungen am Nachbarrepo und gehören nach
  NAK-307 mit diesem abgestimmt.

**Rotbeweis: `szenario.py --selbsttest` (Bein A35), an der Zuordnungszeile.** Neuer
Attrappenfall an `_pruefe`/`_sende` mit der vorhandenen `TestVerbindung`: Schritt A antwortet
mit Timeout, dann mit einer gültigen Peaks-Antwort für `track: 1`; Schritt B fragt `track: 0`
und bekommt von der Attrappe **erneut** die `track: 1`-Antwort. Erwartung: Schritt B ist
**verfehlt**. Vor dem Fix grün (also rot im Sinne des Beweises: der Fall schlägt fehl, weil
der Schritt besteht), nach dem Fix bestanden. Der Rotbeweis fällt an der Zeile, die die
Zusage trägt — `_pruefe` (`:178`) beziehungsweise dem Kennungsvergleich in
`_wait_for_response` — nicht an der Timeoutzählung von M-78. **Kein echter FL-Lauf nötig.**

---

## T3-13-04 — PipeProbe meldet Konfliktende ohne neues Broker-ACK

*Audit: medium · K5 · LÜCKE.*

### 1. Quellkette, zitiert

`eq-copilot/plugin/probe/PipeProbeMain.cpp:135-141` — Stufe 1 verlangt drei Dinge:

```cpp
    // Stufe 1+2: v2 verbunden, mindestens zwei streng passende ACKs. Der
    // Sendezähler allein wäre kein Beweis, solange das zweite ACK aussteht.
    if (! warteAuf (sekunden * 10, [&] {
            const auto z = a->snapshot();
            return z.status == eqcop::PipeClient::Status::verbunden
                && z.protokollVersion == 2 && z.heartbeatsBestaetigt >= 2;
        }))
```

`eq-copilot/plugin/probe/PipeProbeMain.cpp:187-194` — der Abschluss verlangt **nur noch die
Abwesenheit eines Flags**:

```cpp
    b->stop();
    if (! warteAuf (100, [&] { return ! a->snapshot().konflikt; }))
    {
        std::cout << "PROBE FEHLGESCHLAGEN (Konflikt-Ende kam nicht an)" << '\n' << std::flush;
        a->stop();
        return 1;
    }
    std::cout << "KONFLIKT-ENDE OK · Flag faellt nach Trennung des Duplikats" << '\n' << std::flush;
```

`eq-copilot/plugin/src/PipeClient.cpp:344-352` — jeder **Neuaufbau** löscht das Flag:

```cpp
        zustand.status = Status::verbindet;
        zustand.verbindungsVersuche++;
        zustand.brokerVersion.clear();
        zustand.sessionToken.clear();
        zustand.protokollVersion = 0;
        zustand.konflikt = false;
```

`eq-copilot/plugin/src/PipeClient.cpp:625-633` — jedes **Verbindungsende** ebenso:

```cpp
        std::lock_guard<std::mutex> l (zustandMutex);
        zustand.status = Status::getrennt;
        zustand.brokerVersion.clear();
        zustand.sessionToken.clear();
        zustand.protokollVersion = 0;
        zustand.konflikt = false;
```

Zum Vergleich der einzige Ort, an dem das Flag **positiv** gesetzt wird — aus einem
geprüften ACK, `eq-copilot/plugin/src/PipeClient.cpp:591-603`:

```cpp
                const auto konflikt = ack.getProperty ("konflikt", {});
                if (! hatNurFelder (ack, { "type", "seq", "konflikt" })
                    ...
                    || ! konflikt.isBool())
                ...
                zustand.konflikt = static_cast<bool> (konflikt);
```

**Auslöser und Erreichbarkeit:** wie T3-13-01 ein Handaufruf von `eqcop-pipe-probe.exe`
(kein Bein, kein Runner). Innerhalb des 10-Sekunden-Fensters nach `b->stop()` genügt ein
gewöhnlicher Verbindungsabbruch von A — IO-Frist 5 000 ms (`PipeProbeMain.cpp:88`),
Brokerneustart, Pipefehler. **Stufe 1** im selben engen Sinn: Werkzeugeingang ohne
automatischen Aufrufer.

### 2. Zusage, wörtlich

**Keine Zusage gefunden.** Weder das Gate S25e noch eine Matrixzeile in
`docs/beweise/NAK-286.md` noch eine Behauptungszeile in `tools/beweise.ps1` sagt etwas über
das Abschlussprädikat dieses Werkzeugs — folgerichtig, denn es fährt kein Bein. Die einzige
Zusage im Umfeld, `CLAUDE.md:227`, betrifft den Pipenamen, nicht das Urteil.

**Fehlende Regel (Teil 6):** *Ein Konfliktende gilt nur als gemessen, wenn A durchgehend
verbunden geblieben ist und nach dem Stopp von B mindestens ein weiteres ACK derselben
Sitzung ohne Konfliktflag eingetroffen ist; ein Verbindungsabbruch ist kein Konfliktende.*

### 3. Heutige Abdeckung

- **Keine.** Die zwei Quelltextwachen in
  `eq-copilot/plugin/tests/PipeClientLifecycleTestMain.cpp:1428-1452` und folgende prüfen
  Ausnahmegrenze, Exit 70 und Flush — kein Verhalten.
- Bein A4b (`tools/beweise.ps1:505-511`) behauptet über den PipeClient: „stop/reconnect und
  ganze Frames sind zeitlich begrenzt, SQOS ist Identification, Peerbytes sind UTF-8/NUL-sauber,
  ACK-Sequenzen streng und u64-Zaehler wire-sicher." Das Abschlussprädikat des Probewerkzeugs
  ist darin nicht enthalten.
- Der Snapshot trägt alles, was für einen dichten Test gebraucht wird,
  `eq-copilot/plugin/src/PipeClient.h:102-113`: `verbindungsVersuche`, `heartbeatsGesendet`,
  `heartbeatsBestaetigt`, `protokollVersion`, `konflikt`, `letztesAck`.

### 4. Urteil

**BESTÄTIGT, präzisiert.** Die Asymmetrie zwischen `:137-141` und `:188` ist an der Quelle
belegt, und beide Löschstellen des Flags sind vorhanden. Drei Präzisierungen:

1. **Zeilendrift:** das Audit nennt `PipeClient.cpp:626`; die Zuweisung
   `zustand.konflikt = false` steht am HEAD in `:630`, innerhalb des Abbaublocks `:624-634`.
   `:351` stimmt exakt.
2. **Die Lücke ist eng.** Der vorletzte Schritt (`:164`, beide Instanzen sehen das Flag)
   verlangt zwei positiv gesetzte Flags und damit zwei geprüfte ACKs (`:591-603`) — ein
   falsches Grün ist nur im **letzten** Schritt erreichbar.
3. **Der Schaden ist ausschließlich beweisrechtlich.** Das Werkzeug fährt kein Bein; es
   entsteht kein Produktfehler, sondern eine falsche Aussage in einem Handbeleg.

**Kategorievorschlag: LÜCKE, Schwere low** (Herabstufung von medium). Begründung: kein Bein,
kein Runner, kein Produktpfad; die Folge ist ein falsch grüner Handbeleg. Der Dirigent kann
medium halten, wenn er die Handbelege gleich gewichtet.

### 5. Kleinster Fix und Ort des Rotbeweises

Das Abschlussprädikat gegen einen **vor** dem Stopp von B gezogenen Snapshot messen und in
eine reine Funktion ziehen — sinnvollerweise in denselben neuen Kopf wie bei T3-13-01
(`eq-copilot/plugin/src/ProbePipeRegel.h`):

```text
konfliktEndeBestaetigt(vorher, jetzt) :=
       jetzt.status == verbunden
    && jetzt.protokollVersion == 2
    && jetzt.verbindungsVersuche == vorher.verbindungsVersuche
    && jetzt.heartbeatsBestaetigt > vorher.heartbeatsBestaetigt
    && ! jetzt.konflikt
```

`PipeProbeMain.cpp:187-188` zieht vor `b->stop()` einen Snapshot und wartet auf dieses
Prädikat statt auf `! konflikt`.

**Rotbeweis: Bein A4b (`EqCopPipeClientTest`), an der Prädikatzeile, nicht an einem
Nebeneffekt.** Drei synthetische Snapshotpaare im vorhandenen Testmain: (a) verbunden,
`heartbeatsBestaetigt` 5→6, `konflikt` true→false, `verbindungsVersuche` gleich → **bestanden**;
(b) getrennt, `konflikt` true→false, Zähler unverändert → **abgewiesen**; (c) verbunden nach
Neuaufbau (`verbindungsVersuche` 1→2), `konflikt` false → **abgewiesen**. Vor dem Fix sind
(b) und (c) rot, weil die Funktion nicht existiert und das heutige Prädikat beide annimmt.
Die vorhandene Fake-Pipe-Maschinerie des Testmains (`:821-824`, `:58`) erlaubt zusätzlich
einen Lauf mit echtem Abbruch; für den Rotbeweis genügen die Snapshotpaare. **Kein Broker,
kein FL.**

---

## T3-13-05 — Szenarioimport lädt entgegen M-22 indirekt `fl_trigger`

*Audit: medium · K5 · DEFEKT.*

### 1. Quellkette, zitiert

`tools/fl/szenario.py:238-246` — der einzige echte Importweg der Szenarien:

```python
    def verbindung(self):
        try:
            from fl_studio_mcp.utils.connection import get_connection
        except ImportError as e:
            raise SystemExit(
                f"fl_studio_mcp nicht importierbar ({e}); aus dem MCP-Repo starten: "
                "uv run --directory <fl-studio-mcp> python szenario.py ..."
            ) from e
```

`fl-studio-mcp` (HEAD `01f72386`) `src/fl_studio_mcp/utils/__init__.py:1-5` — Python führt
dieses Elternpaket vor `utils.connection` aus:

```python
"""FL Studio MCP utilities."""

from fl_studio_mcp.utils.connection import FLConnection, get_connection, reset_connection
from fl_studio_mcp.utils.fl_trigger import FLStudioTrigger, get_trigger, trigger_fl_studio
```

`tools/fl/szenario.py:2415-2434` — die Wache, beide Hälften:

```python
@fall("M-22", "kein_piano_roll_weg")
def fall_kein_piano_roll_weg(p: Pruefer) -> None:
    import ast

    baum = ast.parse(Path(__file__).read_text(encoding="utf-8"))
    ...
    p(not treffer, f"Piano-Roll-Weg importiert: {treffer}")
    umg = TestUmgebung()
    umg.conn = TestVerbindung(umg)
    fahre_attrappe(umg, {"id": "x", "schritte": [{"aktion": "system.ping"}]})
    geladen = sorted(m for m in sys.modules if "fl_trigger" in m or m.startswith("pynput"))
    p(not geladen, f"nach dem Attrappenlauf geladen: {geladen}")
```

Die erste Hälfte liest die **eigene** AST, die zweite prüft `sys.modules` nach einem Lauf,
der `Umgebung.verbindung` (`:238`) nie betritt — also nie den Import auslöst, den M-22 verbietet.

`src/fl_studio_mcp/utils/fl_trigger.py:64-67` und `:150-158` — die Begrenzung der Tragweite:

```python
    def _trigger_macos_pynput(self) -> bool:
        """Trigger FL Studio on macOS using pynput."""
        try:
            from pynput.keyboard import Controller, Key
```

```python
    def _trigger_windows(self) -> bool:
        """Trigger FL Studio on Windows using pynput.
        ...
            from pynput.keyboard import Controller, Key
```

**Auslöser:** jeder echte Szenarioschritt des Laufzeit-Arms —
`laufzeit.ps1:320-327` startet `szenario.py` über `uv run --directory $McpRepo`, dort ist
das Paket importierbar und `Umgebung.verbindung` wird betreten. **Erreichbarkeit: Stufe 1**,
bei jedem Laufzeitlauf, ohne Sonderbedingung.

### 2. Zusage, wörtlich

Matrixzeile **M-22**, `docs/beweise/NAK-286.md:535`:

> kein Import von `fl_studio_mcp.utils.fl_trigger` und `pynput`; jede FL-Aktion geht über
> `send_command` (MIDI-Trigger und JSON-Datei), keine über einen Tastendruck

Dieselbe Zeile nennt als Test „künftig Selbsttest `kein_piano_roll_weg` (prüft `sys.modules`
nach einem Attrappenlauf), A35" und als Rotbeweis „Import von `fl_trigger` in `szenario.py`
→ Selbsttest rot". Die Zusage ist also **weiter** als der Rotbeweis, den sie sich selbst
gibt: die Zusage spricht vom Import, der Rotbeweis nur vom Import **in der eigenen Datei**.
Darüber steht Gate S25e (2): „der Piano-Roll-Weg (Vordergrund, Tastendruck) bleibt aus der
Automatik draußen."

### 3. Heutige Abdeckung

- Selbsttest `kein_piano_roll_weg` (`tools/fl/szenario.py:2415-2434`), Bein A35 — deckt die
  eigene Datei und einen Attrappenlauf ab, **nicht** den echten Importweg.
- Bein A35, `tools/beweise.ps1:618`, behauptet ausdrücklich, der Selbsttest laufe „ohne FL,
  ohne Installation und **ohne MCP-Repo** gegen Attrappen". Das ist keine Nachlässigkeit,
  sondern eine Eigenschaft des Beins — und es ist der Grund, warum die Wache dort blind
  bleiben **muss**: unter `py -3.13` (`tools/fl/selbsttest.py:38-40`) ist
  `fl_studio_mcp` gar nicht importierbar.
- Im Nachbarrepo gibt es zu dieser Importgrenze **keinen** Test. `tests/` enthält vier
  Dateien — `test_controller_probe_handlers.py`, `test_fenster.py`, `test_mixer_routing.py`,
  `test_port_selection.py`. Am nächsten kommt `tests/test_fenster.py:172`:

```python
    assert "pynput" not in vars(fenster)
```

  Das prüft den **Namensraum eines Moduls**, nicht den Importgraphen des Pakets — und es
  prüft `pynput`, also die Hälfte von M-22, die ohnehin hält.

### 4. Urteil

**BESTÄTIGT, präzisiert.** Der Bruch ist zwingend: Python führt `utils/__init__.py` vor
`utils.connection` aus, und dessen Zeile 4 importiert `fl_trigger`. Drei Präzisierungen:

1. **Nur die erste Hälfte von M-22 bricht.** `pynput` wird **nicht** geladen
   (`fl_trigger.py:67`, `:158` importieren es erst in Methoden), es wird kein Fokus gewechselt
   und keine Taste gedrückt. Die Zusage „keine FL-Aktion über einen Tastendruck" hält; die
   Zusage „kein Import von `fl_studio_mcp.utils.fl_trigger`" hält nicht.
2. **Die Wache ist nicht schlampig, sondern am falschen Ort.** A35 ist per Behauptung
   MCP-frei; eine Prüfung des echten Importgraphen kann dort nicht stattfinden. Ein Fix, der
   nur den Selbsttest erweitert, erzeugt entweder einen still übersprungenen Fall oder bricht
   die A35-Behauptung.
3. **Zeilendrift:** das Audit nennt `tools/fl/szenario.py:241`; die Importanweisung steht am
   HEAD in `:240` (`:241` ist die `except`-Zeile). Harmlos.

**Kategorievorschlag: DEFEKT, Schwere low** (Herabstufung von medium). Begründung: null
Produktwirkung heute; der Schaden ist, dass eine abgenommene Matrixzeile grün gelesen wird,
ohne gemessen zu sein — ein K5-Bruch, kein K2-Bruch. Der Dirigent entscheidet.

### 5. Kleinster Fix und Ort des Rotbeweises

Zwei Hälften, beide klein:

- **Nachbarrepo:** `src/fl_studio_mcp/utils/__init__.py:4` ersatzlos streichen und die drei
  Namen aus `__all__` (`:10-12`) entfernen. Kein Aufrufer im Szenariopfad braucht sie.
- **Nakama:** die Wache dorthin verlegen, wo das echte Paket existiert. `laufzeit.ps1`
  bekommt vor der ersten FL-Aktion einen Schritt, der
  `uv run --directory $McpRepo --python 3.12 python <szenario.py> --selbsttest --nur kein_piano_roll_weg`
  fährt (dieselbe Fassade wie `:313` und `:325`); der Fall selbst bekommt eine dritte Hälfte,
  die `Umgebung().verbindung()` in einem Unterprozess betritt und `sys.modules` danach prüft.
  Ist das Paket nicht importierbar (A35-Weg), meldet der Fall **„nicht gemessen"**, nie grün.

**Rotbeweis: zweistufig, je an der Zeile, die die Zusage trägt.**

1. **Bindend, im Nachbarrepo:** neuer Pytest `test_connection_import_does_not_load_trigger`
   in `tests/`, der in einem frischen Interpreter
   `importlib.import_module("fl_studio_mcp.utils.connection")` ausführt und prüft, dass
   weder `fl_studio_mcp.utils.fl_trigger` noch ein `pynput`-Modul in `sys.modules` steht.
   **Am MCP-HEAD `01f72386` rot**, nach dem Streichen von `:4` grün. Das ist der Rotbeweis an
   der Zeile, die M-22 trägt.
2. **Ergänzend, in Nakama:** die dritte Hälfte von `kein_piano_roll_weg` unter dem
   MCP-Kontext. Vor dem Fix rot (die Attrappenumgebung wird verlassen, das echte Paket
   geladen, `fl_trigger` erscheint in `sys.modules`); im A35-Weg ohne MCP-Repo meldet der
   Fall „nicht gemessen" statt grün — was **heute schon** eine Abweichung ist und die
   Blindheit der Wache sichtbar macht.

Für `laufzeit.ps1 -Selbsttest` fällt hier **kein** Attrappenfall an; der neue Runnerschritt
ist ein reiner `uv run`-Aufruf und wird über die vorhandene Attrappe `FahreSzenario`
(`:320`) im Protokoll geprüft. **Kein echter FL-Lauf.**

---

## T3-13-06 — Externe MCP-Arbeitskopie ohne geprüften Commit-Pin

*Audit: low · K5 · HÄRTUNG.*

### 1. Quellkette, zitiert

`tools/fl/laufzeit.ps1:64` — die Fremdquelle kommt als Pfad, ohne Stand:

```powershell
    [string]$McpRepo = 'C:\Users\phili\Projekte\fl-studio-mcp',
```

`tools/fl/laufzeit.ps1:308-315` — beide Ausführungswege nehmen den **Arbeitsbaum**, wie er
gerade ist:

```powershell
    $u.SetupLocal = {
        $zeilen = @(& pwsh -NoProfile -File (Join-Path $script:K.McpRepo 'setup-local.ps1') 2>&1 | ForEach-Object { "$_" })
        @{ Exit = $LASTEXITCODE; Zeilen = $zeilen }
    }
    $u.Ping = {
        $ausgabe = & uv run --directory $script:K.McpRepo --python 3.12 python $script:K.SzenarioPy --ping 2>&1
```

`tools/fl/laufzeit.ps1:325` — dasselbe für jeden Szenariolauf:

```powershell
        $zeilen = @(& uv run --directory $script:K.McpRepo --python 3.12 python @argumente 2>&1 | ForEach-Object { "$_" })
```

`tools/fl/laufzeit.ps1:434-442` — `Pruefe-Controller` ist eine echte Wache, aber für **eine**
Datei:

```powershell
    $repoSkript = Join-Path $K.McpRepo 'fl_controller\device_FLStudioMCP.py'
    if (-not (& $script:U.Existiert $repoSkript)) { throw "Controller-Skript fehlt im MCP-Repo: $repoSkript" }
    ...
    Log ("Controller-Skript: SHA-256 Repo {0}, installiert {1} ({2}), script_version {3}" -f $hRepo, $hInstalliert, $installiert, $K.ScriptVersion)
    if ($hRepo -eq $hInstalliert) { return }
```

`tools/fl/laufzeit.ps1:121-127` — was das Rohprotokoll festhält:

```powershell
function Schreibe-Roh([string]$urteil, [string]$zusatz) {
    $K = $script:K
    $kopf = @(
        "# Laufzeit-Arm $($K.Ticket) - $urteil",
        "",
        "Zeit: $(Zeit) | Basis: $($K.Basis) | HEAD: $($K.Head) | Repo: $($K.Repo) | Projekt: $($K.ProjektName)",
```

`tools/fl/laufzeit.ps1:1601` — `HEAD` ist ausschließlich der Nakama-Stand:

```powershell
$kopfSha = (& git -C $Repo rev-parse --short=8 HEAD 2>$null)
```

Dass Git nie gegen das Fremdrepo läuft, steht in der Fassade selbst,
`tools/fl/laufzeit.ps1:251-255`:

```powershell
    $u.Git = {
        param([string[]]$argumente)
        $zeilen = @(& git -C $script:K.Repo @argumente 2>&1 | ForEach-Object { "$_" })
        @{ Exit = $LASTEXITCODE; Zeilen = $zeilen }
    }
```

**Auslöser:** jeder Laufzeitlauf. **Erreichbarkeit: Stufe 1** (das Rohprotokoll jedes Laufs
ist bereits so entstanden, siehe `docs/beweise/roh/NAK-286-laufzeit-34bdf159-lauf1.md`).

### 2. Zusage, wörtlich

**Keine Zusage gefunden.** Weder das Gate S25e noch eine Matrixzeile in
`docs/beweise/NAK-286.md` noch `tools/fl/LIES-MICH.md` verlangt einen unveränderlichen
externen Stand. `tools/fl/LIES-MICH.md:76-79` nennt nur die Herkunft:

> das MCP-Repo unter dem Standardpfad
> (`-McpRepo` sonst): privates Repo `https://github.com/evenacadia-tech/fl-studio-mcp`,
> Zweig `evenacadia-local`, nach `C:\Users\phili\Projekte\fl-studio-mcp` klonen
> und dort einmal `setup-local.ps1` ausführen

**Fehlende Regel (Teil 6):** *Eine Hostmessung, die als reproduzierbar gilt, nennt in ihrem
Rohprotokoll den Stand jeder Fremdquelle, die an ihr beteiligt war, und weist ihn gegen einen
im Nakama-Repo abgelegten, datiert geänderten Sollstand ab.*

### 3. Heutige Abdeckung

- `Pruefe-Controller` (`:428-455`) hasht `fl_controller/device_FLStudioMCP.py` auf beiden
  Seiten, protokolliert beide Werte und die `script_version` (`:441`) und gleicht bei
  Abweichung über `setup-local.ps1` an — **eine** Datei des Fremdbaums ist damit
  fingerabdruckt. Matrixzeile M-09 (`docs/beweise/NAK-286.md:516`) deckt genau das ab.
- Selbsttest `controller_veraltet_neu_installieren` (`tools/fl/laufzeit.ps1:1228`), Bein A35.
- Für **alle übrigen** Dateien des Fremdbaums — `utils/midi_connection.py`,
  `utils/connection.py`, `utils/fenster.py`, `utils/__init__.py` — gibt es keinen
  Fingerabdruck und keinen Sollstand.
- Registerzeile NAK-286 verweist auf das Fremdrepo, hält aber keinen erlaubten Stand fest.

### 4. Urteil

**BESTÄTIGT, präzisiert.** Die Kette hält an der Quelle: Pfad ohne Stand (`:64`), zwei
Ausführungswege über den Arbeitsbaum (`:313`, `:325`), Fingerabdruck nur für eine Datei
(`:434-441`), Rohkopf ohne Fremdstand (`:126`, `:1601`), kein Git gegen das Fremdrepo
(`:251-255`). Zwei Präzisierungen:

1. **„ohne Bibliotheksstand" ist zu breit.** Das Nachbarrepo führt eine `uv.lock`; für einen
   gegebenen Arbeitsbaum sind die **Drittbibliotheken** damit deterministisch. Unidentifiziert
   bleiben (a) der Commit des Arbeitsbaums, (b) ob er schmutzig ist, (c) die Identität der
   `uv.lock` selbst. Der eigentliche blinde Fleck ist also der **Erstcode** des MCP-Repos
   (`midi_connection.py`, `fenster.py`, `utils/__init__.py`) — genau die Dateien, an denen
   T3-13-03 und T3-13-05 hängen.
2. **`Pruefe-Controller` ist keine Alibiwache.** Sie ist echt, misst aber die falsche Grenze:
   sie vergleicht Repo gegen **Installation**, nicht Repo gegen **Sollstand**.

**Kategorievorschlag: HÄRTUNG, Schwere low (bestätigt).** Der Nutzen ist genau der vom Audit
genannte: eine Hostmessung ihrer Gegenseite zuordnen können. Der Zusammenhang mit T3-13-03
und T3-13-05 erhöht den Wert: ohne Pin kann niemand nachträglich sagen, ob eine Messung vor
oder nach einer Reparatur im Nachbarrepo entstanden ist.

### 5. Kleinster Fix, Ort des Pins und Ort des Rotbeweises

**Wo der Pin festgehalten und geprüft werden sollte, ohne das Nachbarrepo zu ändern** (Vorschlag):
eine neue Datei **im Nakama-Repo**, `tools/fl/mcp-stand.json`, mit

- `revision`: der erlaubte Commit des Zweigs `evenacadia-local` (heute
  `01f72386bfc7eab507a40b9c96ef9e2e2a0265cc`),
- `sha256`: eine kleine Tabelle der Dateien, an denen Nakama-Zusagen hängen —
  `fl_controller/device_FLStudioMCP.py`, `src/fl_studio_mcp/utils/midi_connection.py`,
  `src/fl_studio_mcp/utils/connection.py`, `src/fl_studio_mcp/utils/__init__.py`,
  `src/fl_studio_mcp/utils/fenster.py`, `uv.lock`,
- `stand_vom` mit Datum und Ticket.

`laufzeit.ps1` liest sie vor dem ersten `uv run` (also vor `:313`), misst `git -C $McpRepo
rev-parse HEAD` und `git -C $McpRepo status --porcelain` sowie die sechs SHA-256, schreibt
**alle** gemessenen Werte in den Rohkopf (`:126`) und endet bei Abweichung mit Exit 3
`VORAUSSETZUNG`, der Sollwert und Istwert nennt. Das Nachbarrepo wird dabei nur gelesen. Die
Datei zu aktualisieren ist ein eigener, datierter Nakama-Commit — nie eine stille Übernahme;
genau das unterscheidet den Pin von der heutigen Lage.

**Rotbeweis: Attrappenfall in `laufzeit.ps1 -Selbsttest` (Bein A35), an der Abgleichzeile.**
Die vorhandene Fassade kennt bereits `$u.Git` (`:251`) und `$u.Hash`; der neue Fall
`Fall 'M-09' 'mcp_stand_gepinnt'` fährt drei Attrappen:

- Ist-Revision gleich Soll, alle Hashes gleich, Arbeitsbaum sauber → Lauf geht weiter, und
  der Rohkopf enthält Revision und Hashes (`T-Hat` auf die Kopfzeile).
- Ist-Revision abweichend → Exit 3 `VORAUSSETZUNG` mit Soll und Ist im Grund.
- Ein Dateihash abweichend bei gleicher Revision (schmutziger Arbeitsbaum) → Exit 3.

Vor dem Fix sind alle drei rot: es gibt keine Solldatei, keinen Abgleich und keinen
Fremdstand im Rohkopf. Der Rotbeweis fällt an der Abgleichzeile, nicht am
Controller-SHA-Vergleich von M-09, der unberührt weiterläuft. **Kein echter FL-Lauf, kein
`uv run`** — die Git- und Hashzugriffe laufen über die Attrappenfassade.

---

## Nebenbefunde (nicht Teil der IDs, keine neuen IDs)

- **NB-1 — `eq-copilot/README.md:61` beschreibt einen Ablauf, der seit NAK-123 scheitern
  muss.** Das Beispiel ruft `EqCopPipeProbe.exe "\\.\pipe\evenacadia.eq-copilot.m2probe"`
  **ohne** drittes Argument. Ohne dieses Argument bleibt der installierte Broker die
  Erwartung (`PipeProbeMain.cpp:55-59`), während auf der Probe-Pipe
  `eqcop-broker-probe.exe` lauscht — also `dateiidentitaetFalsch` vor dem ersten Byte. Genau
  diesen Bruch hat NAK-123 P2 behoben (`docs/beweise/NAK-123.md:482-484`) und in
  `docs/plugin-wissen.md:881-886` nachgezogen; `eq-copilot/README.md` blieb zurück. Betrifft
  dieselbe Datei wie T3-13-01 und gehört in denselben Änderungssatz.
- **NB-2 — Die Behauptungszeile von Bein A35 (`tools/beweise.ps1:618`) ist für zwei meiner
  IDs zu weit.** Sie behauptet „beendet nie ein fremdes FL" (T3-13-02 widerlegt das für den
  namensgleichen Fall) und deckt M-22 ab, ohne den echten Importweg messen zu können
  (T3-13-05). Die Zeile ist im selben Änderungssatz nachzuziehen, sonst behauptet der Kanon
  weiter mehr, als er misst.
- **NB-3 — `Warte-Ping` (`tools/fl/laufzeit.ps1:679-683`) setzt `$K.DiagnosePid` auf die
  erste Titelübereinstimmung**, auch wenn `Starte-FL` (`:646`) zuvor eine andere PID gesetzt
  hat. Dieselbe fehlende Besitzbindung wie T3-13-02, aber mit anderer Folge: die Messung kann
  auf die falsche Instanz zeigen, ohne dass etwas beendet wird. Der Fix aus T3-13-02 schließt
  das mit; ich führe es nur auf, damit es beim Bau nicht vergessen wird.
- **NB-4 — Der Kanon baut `EqCopPipeProbe` nicht** (`tools/beweise.ps1:883-884`). Wer den
  Fix aus T3-13-01 oder T3-13-04 baut und nur den Kanon fährt, hat das geänderte Werkzeug
  nicht übersetzt. Der Rotbeweis beider IDs liegt deshalb bewusst in A4b, das gebaut wird —
  ein eigener Bauaufruf für `EqCopPipeProbe` gehört trotzdem in die Abschlussprüfung des
  Tickets.

---

## Scope-Beweis

**Gelesene Dateien, Nakama (HEAD `4c1c7f3c`, unverändert auch an `3b4e8bd0`):**

| Datei | Zeilen gesamt | gelesen |
| --- | --- | --- |
| `CLAUDE.md` | — | vollständig (Daueranweisung) |
| `tools/fl/LIES-MICH.md` | 146 | vollständig |
| `tools/fl/laufzeit.ps1` | 1613 | Abschnitte 55-70, 118-135, 230-270, 305-375, 396-450, 558-575, 642-720, 775-845, 907-918, 1063-1117, 1119-1130, 1592-1613 sowie Trefferlisten über alle Aufrufstellen |
| `tools/fl/szenario.py` | 3492 | Abschnitte 38-70, 110-190, 225-265, 608-643, 2395-2440 |
| `tools/fl/selbsttest.py` | 82 | 1-50 |
| `tools/beweise.ps1` | 1447 | Abschnitte 500-520, 610-620, 860-890 sowie Trefferlisten |
| `eq-copilot/plugin/probe/PipeProbeMain.cpp` | 213 | vollständig |
| `eq-copilot/plugin/src/EqCopilotIds.h` | 26 | vollständig |
| `eq-copilot/plugin/src/PipeClient.cpp` | 824 | 200-240, 335-370, 605-660 sowie alle `konflikt`-Fundstellen |
| `eq-copilot/plugin/src/PipeClient.h` | 174 | 95-125 |
| `eq-copilot/plugin/core/ipc/PipeToken.h` | 88 | 30-79 |
| `eq-copilot/plugin/CMakeLists.txt` | 1386 | 594-624, 1344-1360 |
| `eq-copilot/plugin/tests/PipeClientLifecycleTestMain.cpp` | 1555 | 1400-1470 |
| `eq-copilot/README.md` | 119 | 50-70 |
| `docs/plugin-wissen.md` | 1000 | 875-890 |
| `docs/plan/plan.json` | — | Schritt `S25e` vollständig |
| `docs/beweise/NAK-286.md` | 1466 | 508-535, 1093, 1375 sowie Trefferlisten |
| `docs/beweise/NAK-289.md` | — | 330-345, 505-525 |
| `docs/beweise/NAK-123.md` | — | Trefferlisten zu `EqCopPipeProbe` |
| `docs/offene-punkte.md` | 514 | Zeilen 293, 300, 315, 316, 513 |
| `docs/gesundheit/KONZEPT.md` | 760 | 127-175 (§3, K1-K8) |
| `docs/gesundheit/szenarien/bereitschaft.json` | 21 | vollständig |
| `docs/audits/2026-09-15-tiefenaudit/BEFUNDE.md` | 840 | 560-575, 790-830 |
| `docs/audits/2026-09-15-tiefenaudit/roh/phase-13-bauhostkette.md` | 834 | 14-80, 150-200, 620-670 |
| `docs/beweise/roh/NAK-246-quellvalidierung.md` | 835 | Kopf und Befund D1 (Musterübernahme) |
| `docs/beweise/roh/NAK-283-quellvalidierung.md` | 2722 | Kopf und Befund F01 (Musterübernahme) |
| `docs/beweise/roh/NAK-286-laufzeit-34bdf159-lauf1.md` | — | Zeile 49 (gemessener Fenstertitel) |
| `docs/beweise/NAK-286-verlauf.md` | — | Trefferlisten zu `TFruityLoopsMainForm` |

**Gelesene Dateien, `C:\Users\phili\Projekte\fl-studio-mcp` (HEAD `01f72386`, nur lesend):**

| Datei | Zeilen gesamt | gelesen |
| --- | --- | --- |
| `src/fl_studio_mcp/utils/__init__.py` | 13 | vollständig |
| `src/fl_studio_mcp/utils/connection.py` | 93 | vollständig |
| `src/fl_studio_mcp/utils/midi_connection.py` | 320 | 175-284 |
| `src/fl_studio_mcp/utils/fl_trigger.py` | 239 | 1-60 sowie alle Import- und `pynput`-Fundstellen |
| `fl_controller/device_FLStudioMCP.py` | 1203 | 108-188, 725-745 |
| `tests/test_fenster.py` | — | Zeile 172 (`pynput`-Wache) |
| `uv.lock`, `pyproject.toml` | — | nur Existenz geprüft, Inhalt nicht gelesen |
| `tests/`, `src/fl_studio_mcp/utils/` | — | nur Verzeichnislisten |

**Ausdrücklich nicht gelesene Pfade:** `broker/` (gesamter Rust-Baum), `eq-copilot/install/`,
`eq-copilot/schemas/`, `eq-copilot/fixtures/`, `design/`, `wissen/`, die Befunde
T3-12-01 bis T3-12-05 (Sicherheit, ausgeschlossen), alle übrigen Phasen des Tiefenaudits
außer Phase 13 und der Arbeitsübersicht V1, `docs/FL-Nakama-Sonden-Design-Entwurf.md`,
`tools/fl/nulltest.py`, `tools/plan/`, `tools/pruefung/`, sowie im Nachbarrepo die übrigen
Testdateien, `scripts/`, `assets/`, `install.ps1`, `install.sh`, `setup-local.ps1`,
`src/fl_studio_mcp/utils/fl_paths.py` und der MCP-Serverteil unter `src/fl_studio_mcp/`
außerhalb von `utils/`.

**Nicht ausgeführt:** kein Bau, kein Test, kein Kanon, kein Selbsttest, kein FL-Start, kein
`uv run`, kein MCP-Aufruf, kein Codex, kein `git add`, kein Commit, kein Push. Einzige
Schreibaktion: diese Datei.
