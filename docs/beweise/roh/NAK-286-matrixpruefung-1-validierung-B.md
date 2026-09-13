# NAK-286 — Validierung der Codex-Matrixprüfung 1, Teil B (lesend, 14.09.2026)

Ticket NAK-286 (Plan S25e, Etappe 1). HEAD `a4c7128b`. Teil B der drei
Validierungen: **Befunde 2, 3, 4, 10, 13** (Runner, Exitcodes, MCP). Nur
gelesen — nichts gebaut, nichts ausgeführt, kein FL gestartet, Git nur lesend;
geschrieben wurde ausschließlich diese Datei. Das MCP-Repo
`C:\Users\phili\Projekte\fl-studio-mcp` wurde nur gelesen (Stand `29f6656`,
Zweig `evenacadia-local`; unversioniert nur zwei `.pyc` unter
`tests/__pycache__/`).

**Zeilenversatz: null.** Das Urteil zitiert `docs/beweise/NAK-286.md` zum Stand
`0523c079`. `git diff 0523c079..HEAD -- docs/beweise/NAK-286.md` ergibt
15 Einfügungen und 1 Löschung: die Kopfzeile „Etappe" (Zeile 10) wird an Ort
und Stelle ersetzt, alles Weitere wird hinter Zeile 929 als §9 angehängt. Vor
§9 verschiebt sich keine Zeile. Alle Zitate des Prüfers treffen an HEAD
dieselbe Zeile; geprüft an den fünf zitierten Stellen: `:571` = M-65,
`:577` = M-66, `:378/:380` = F-17/F-19, `:495` = M-20, `:476` = M-06.

---

## Befund 2 [P1] — „Beende Diagnose-FL vor dem Rückweg"

**Wörtlich (Urteil):** `[P1] … NAK-286.md:571-571 — DEFEKT M-65/§5.7: Beim Ende
von `nulltest-host` läuft das interaktive Diagnose-FL bereits;
`tools/fl/laufzeit.ps1:306–322` startet es vor den Szenarien, und selbst
`-Beenden` greift erst danach bei Zeile 326. Der Rückweg verweigert bei jedem
laufenden FL ausdrücklich sämtliche Änderungen
(`eq-copilot/install/Install-Nakama.ps1:651`). Ohne vorgeschaltetes Beenden
bleibt somit gerade der als abweichend erkannte Bau installiert. Matrix und
Aufgaben-Selbsttest müssen die Reihenfolge Diagnose-FL beenden → Rückweg →
Prüfen verlangen, entsprechend [CLAUDE.md:225–226].`

**Quelle an HEAD — `tools/fl/laufzeit.ps1:306-326`:**

```powershell
try {
    Stelle-LoopMidi-Sicher
    Beende-Restprozesse
    $flp = Bereite-Projekt
    $laufend = @(FL-Instanzen | Where-Object { Ist-Diagnose $_ })
    …
    if ($laufend.Count -eq 0) { $null = Starte-FL $flp; Warte-Boot } else { … }
    $ping = Warte-Ping
} catch { … }

Schreibe-Roh 'LAEUFT' "installation=$installation"
$ergebnisse = Fahre-Szenarien
…
if ($Beenden) { Beende-Diagnose-FL }
```

Zwischen dem FL-Start (`:314`) und dem Ende der Szenarien (`:322`) beendet der
Runner kein FL. `-Beenden` wirkt erst in `:326`, also **nach** allen Szenarien;
M-65 verlangt den Rückweg aber „direkt nach dem Szenario, vor weiteren
Szenarien".

**Quelle an HEAD — `eq-copilot/install/Install-Nakama.ps1:651` im
`-Rueckweg`-Zweig:**

```powershell
    if (Fl-Laeuft) { Abbruch 'FL Studio laeuft. Der Rueckweg fasst nichts an.' }
```

`Fl-Laeuft` (`:538-540`) prüft **jeden** Prozess, nicht nur ein Fenster:

```powershell
function Fl-Laeuft {
    $null -ne (Get-Process -Name 'FL64', 'FL', 'FL Studio' -ErrorAction SilentlyContinue)
}
```

Die Prüfung steht **vor** dem NAK-41-Riegel (`:810`); der Abbruchtext lautet
also „FL Studio laeuft. Der Rueckweg fasst nichts an.", nicht die von M-65
erwartete NAK-41-Meldung. Die Aufgabe `\Nakama\rueckweg` ist genau dieser
Aufruf (`tools/dirigent/install-aufgaben-registrieren.ps1:48`:
`rueckweg = ' -Rueckweg'`). Die Aufgabe `\Nakama\pruefen` (`-Pruefen`) ist von
dem Riegel nicht betroffen: sie endet mit `exit 0` vor Riegel 3 (`:962`) und
meldet dann folgerichtig „aktuell" für den abweichenden Bau.

**Gegenstand — Matrixzeile M-65, HEAD-Zeile 571:**

> „der Runner startet `\Nakama\rueckweg`, danach `\Nakama\pruefen`, und
> schreibt beide Logs roh; das Urteil bleibt `VERFEHLT` (Exit 4); verweigert
> der Rückweg (NAK-41-Riegel, `eq-copilot/install/Install-Nakama.ps1:37-39`),
> steht „Rückweg verweigert" in der Kopfzeile … **direkt nach dem Szenario, vor
> weiteren Szenarien**"

und Bauplan §5.7 Punkt 1 (HEAD-Zeile 788), der die Reihenfolge nicht nennt:

> „… `frischer_start` (M-59); **Rückweg bei ABWEICHUNG (M-65)**; Schalter
> `-Selbsttest` mit Attrappen für FL-Instanzen, Aufgaben, `py`, `git`, `FL64`
> und Dateien"

Ebenso schweigt F-20 (HEAD-Zeile 381): „Nur ABWEICHUNG löst `\Nakama\rueckweg`
und danach `\Nakama\pruefen` aus".

**Status: BESTÄTIGT.** Zum Zeitpunkt, den M-65 nennt, läuft das Diagnose-FL
zwangsläufig — der Runner hat es selbst gestartet und beendet es vor `:326`
nirgends. `Fl-Laeuft` erfasst es, der Rückweg bricht ab, und der als abweichend
erkannte Bau bleibt installiert. Damit ist die Zusage der Zeile in **jedem**
Lauf unerfüllbar, nicht nur in einem Randfall. Verschärfend:

- Der zugesagte Rotbeweis („Selbsttest `rueckweg_nur_bei_abweichung`
  (Aufgaben-Attrappe)") läuft gegen eine Attrappe der Aufgabe und wäre grün,
  während der echte Pfad immer abbricht — **der Beweisweg kann die Zusage nicht
  tragen**, gegen den Leitsatz „je Matrixzeile fällt der Rotbeweis an der Zeile,
  die die Zusage trägt" (§2, HEAD-Zeile 345).
- Der Zweck des automatischen Rückwegs steht in §7.1 T-12 (HEAD-Zeile 883):
  „Eine Abweichung im Auslieferungszustand heißt, dass der installierte Bau das
  Grundgesetz im echten Host bricht; ihn bis zum Lesen durch einen Menschen
  stehen zu lassen, duldete ungefragte Verarbeitung auf dem Arbeitsrechner."
  Genau dieses Sicherheitsventil zündet nie.
- Der von M-65 vorgesehene Text „Rückweg verweigert" träfe zu, wäre aber
  irreführend begründet (NAK-41 statt „FL läuft"), und `\Nakama\pruefen` danach
  meldete „aktuell" — eine UI-/Protokollaussage, die lügt.
- Die Zeile ist in §4.12 (HEAD-Zeile 621) als Beleg der Beziehung
  „installieren↔Rückweg" geführt; die Beziehung ist damit nicht abgesichert
  (`CLAUDE.md:84-86`).

**Einordnung (Vorschlag): DEFEKT.** Die Zusage verletzt die Beziehungsregel
`CLAUDE.md:84-86` und das Grundgesetz-Argument aus T-12, und ihr Beweisweg
(Attrappe) kann sie nicht tragen.

**Schließende Änderung (Vorschlag):** M-65 und §5.7 Punkt 1 schreiben die
Reihenfolge fest: **Diagnose-FL beenden (nie ein fremdes, nie speichern) →
`\Nakama\rueckweg` → `\Nakama\pruefen`**, mit `Beende-Diagnose-FL` plus Warten
auf Prozessende als erstem Schritt und einer Messung „kein FL64-Prozess mehr"
vor dem Aufruf. Der Selbsttest `rueckweg_nur_bei_abweichung` bekommt einen
zweiten Fall: eine FL-Attrappe, die während des Rückwegaufrufs lebt, muss den
Schritt rot machen (Reihenfolgeprotokoll der Attrappen). Die Kopfzeile
unterscheidet „Rückweg verweigert (FL läuft)" von „(NAK-41)". Weil das
Beenden des FL die Folgeszenarien (`snapshot-runde01`, `u40-…`) ihres Hosts
beraubt, muss dieselbe Zeile sagen, was mit ihnen geschieht — die Frage gehört
mit Befund 3 in einen Auftrag.

---

## Befund 3 [P2] — „Lasse KETTE-Messungen die Folgeszenarien erreichen"

**Wörtlich (Urteil):** `[P2] … NAK-286.md:577-577 — DEFEKT M-63/M-66/F-26:
KETTE beendet `nulltest-host` mit Exit 3. M-10 hält jedoch ausdrücklich den
sofortigen Abbruch aller weiteren Szenarien fest, den
`tools/fl/laufzeit.ps1:286` implementiert. In der vorgeschriebenen
alphabetischen Reihenfolge werden damit weder die KETTE-Auswertung von M-66
noch U40 erreicht. Der isolierte Selbsttest von M-66 verdeckt diesen
Widerspruch. Der Ablauf muss die zugesagten Folgemessungen auch für KETTE
ermöglichen, während das Gesamturteil wegen des fehlenden Nulltestbeweises
weiterhin 3 bleibt.`

**Quelle an HEAD — `tools/fl/laufzeit.ps1:276-289`:**

```powershell
function Fahre-Szenarien {
    if (-not $Szenarien) { $Szenarien = @(Get-ChildItem (Join-Path $Repo 'docs\gesundheit\szenarien\*.json') | Sort-Object Name | ForEach-Object { $_.FullName }) }
    …
    foreach ($s in $Szenarien) {
        $ausgabe = & uv run --directory $McpRepo --python 3.12 python $szenarioPy $s --roh $Roh 2>&1
        $code = $LASTEXITCODE
        …
        if ($code -eq 3) { break }
    }
```

und das Gesamturteil `:329-331`:

```powershell
if ($ergebnisse | Where-Object { $_.Exit -eq 3 }) { Ende 3 'VORAUSSETZUNG' $zusatz }
if ($verfehlt.Count -gt 0) { Ende 4 'VERFEHLT' $zusatz }
Ende 0 'GEMESSEN' $zusatz
```

**Gegenstand.** M-63 (HEAD-Zeile 569): „Urteil KETTE mit g und 20·log10 g;
Szenario **Exit 3** „Voraussetzung Weg R1 fehlt: FL-Kette nicht
einheitsverstärkend"". M-10 (HEAD-Zeile 480): „Exit 3, sobald eines mit 3 endet
— **die übrigen Szenarien entfallen dann**", Spalte Test: „gebaut". F-26
(HEAD-Zeile 397): „Der Runner fährt die Szenarien alphabetisch
(`tools/fl/laufzeit.ps1:277`): `bereitschaft.json`, `fenster.json`,
`nulltest-host.json`, `snapshot-runde01.json`, `u40-aktivitaetsgate.json`".
M-66 (HEAD-Zeile 577): „Δ_K = 0,0 dB bei BITIDENTISCH; **Δ_K = 20·log10 g bei
KETTE g**". §7.1 T-19 (HEAD-Zeile 891): „Die alphabetische Folge des Runners
ergibt die Abhängigkeitsfolge (Nulltest vor Snapshot); keine Nummernpräfixe,
keine neue Ordnungslogik."

**Status: BESTÄTIGT.** Die Kette ist geschlossen und widerspruchsfrei
nachvollziehbar: `nulltest-host.json` steht alphabetisch an dritter Stelle,
liefert bei KETTE Exit 3, und `:286` bricht die Schleife ab — `snapshot-runde01`
und `u40-aktivitaetsgate` laufen nie. M-66 ist damit für seinen zweiten,
ausdrücklich ausformulierten Fall (KETTE g) tot; sein Rotbeweis („KETTE-Attrappe
mit g = 0,5 … verfehlt das unverschobene Band") fällt im Selbsttest an einer
Attrappe und verdeckt genau das. Zwei Punkte gehen über das Urteil hinaus:

1. **KETTE ist der wahrscheinliche, nicht der seltene Ausgang.** §0.5 Punkt 2
   (HEAD-Zeilen 241-249) misst: „Insert 1 meldet beim Abspielen `peak_left`
   0,49495 = −6,11 dBFS …; die Offline-TP der Datei ist −7,87 dBTP …. Entweder
   hebt die Kette um mindestens 1,76 dB …". Der Bauplan führt das selbst als
   Risiko (b) (HEAD-Zeile 805): „Die Kettenverstärkung aus §0.5 Punkt 2 →
   KETTE, Karte K-286-1." Die Matrix plant also mit hoher Wahrscheinlichkeit
   einen Lauf, in dem die beiden Kernmessungen des Tickets ausfallen.
2. **Der Plan-BEWEIS hängt daran.** Gate (Kopfzeile 8): „BEWEIS:
   `docs/beweise/NAK-286.md` mit Bereitschaftslauf, Nulltest im echten Host,
   **erster U40-Messung mit Rohdaten**". `u40-aktivitaetsgate.json` ist die
   letzte Datei der alphabetischen Folge und fällt bei jedem Exit 3 davor aus —
   auch bei einem Formatfehler (M-61) und bei einer verletzten
   Clip-Voraussetzung (F-17, Befund 4).

Die Wurzel ist, dass Exit 3 zwei verschiedene Dinge bezeichnet: „diesem
Szenario fehlt eine Voraussetzung" (M-61, M-63, M-66 ohne Urteil) und „dem Lauf
fehlt eine Voraussetzung, weitermachen ist sinnlos" (kein Port, kein Ping,
`tools/fl/szenario.py:126-127`). Nur der zweite Fall rechtfertigt `:286`.

**Einordnung (Vorschlag): DEFEKT.** M-10 und M-66 sagen für denselben Ablauf
Unvereinbares zu; T-19 stützt die Abhängigkeitsfolge auf eine Reihenfolge, die
der Abbruch aufhebt; der Beweisweg von M-66 (isolierte Attrappe) kann die
Zusage nicht tragen.

**Schließende Änderung (Vorschlag):** Voraussetzung des Szenarios von
Voraussetzung des Laufs trennen. Additiv im Sinn von R-286-5 (Exitcodes 0/3/4
und Urteilswörter bleiben unangetastet): `nulltest.py`/`szenario.py` melden die
Szenario-Voraussetzung über einen **neuen** Exitcode (etwa 5) plus `ergebnis.json`;
der Runner setzt daraufhin das Gesamturteil auf `VORAUSSETZUNG` (Exit 3),
**bricht die Schleife aber nicht ab**, und behält den Abbruch für die harten
Fälle (kein Port, kein Ping) bei Exit 3. M-10 bekommt den neuen Code in Zusage,
Test (`szenario_exitcodes`, Attrappen 0, 2, 3, 4, 5) und Rotbeweis; M-63 nennt
den neuen Code; M-66, M-67 und M-68 sagen ausdrücklich, welche ihrer Zusagen
unter Δ_K aus KETTE gelten. Ein Selbsttest `kette_erreicht_folgeszenarien`
(Attrappenfolge mit KETTE an Position 3) trägt die Zusage. Zusammen mit
Befund 2 zu klären: bei ABWEICHUNG endet der Host-Betrieb (FL wird für den
Rückweg beendet) — die Folgeszenarien entfallen dort begründet und mit Meldung,
nicht still.

---

## Befund 4 [P2] — „Trenne die Clip-Voraussetzung von einer Audioabweichung"

**Wörtlich (Urteil):** `[P2] … NAK-286.md:380-380 — DEFEKT F-17/F-19/M-62: Für
den ausdrücklich vorgesehenen Fall „Clip nicht ab Songbeginn" verlangt F-17
(Zeile 378) Exit 3. Ein ansonsten identisches, um ein Frame verschobenes Render
hat nach M-62(b) aber einen nichtnulligen Versatz und Sampleabweichungen, die
kein einzelner Faktor erklärt; F-19 klassifiziert es damit als ABWEICHUNG und
M-65 löst den Rückweg aus. Es fehlt eine Entscheidung samt Test, die eine
verletzte Clip-Voraussetzung von tatsächlicher Plugin-Latenz unterscheidet,
bevor dem installierten Bau eine Audioabweichung zugeschrieben wird.`

**Gegenstand — F-17, HEAD-Zeile 378:**

> „Die Voraussetzungen (einheitsverstärkende FL-Kette, verlustfreies
> Renderformat, **Clip ab Songbeginn**) werden **gemessen, nicht angenommen**;
> fehlt eine, endet der Lauf **Exit 3** mit dem gemessenen Grund"

**F-19, HEAD-Zeile 380:**

> „**Versatzsuche ±4 410 Frames** an einem 64-Frame-Block ohne Stille; erwartet
> Versatz 0. Urteile: **BITIDENTISCH** (0 Abweichungen, Versatz 0); **KETTE g**
> (alle Abweichungen erklärt ein einziger Faktor g bis auf Float-32-Rundung;
> Exit 3); **ABWEICHUNG** (sonst; Exit 4)."

**M-62 Rotbeweis, HEAD-Zeile 568:** „(b) Versatz 1 → Versatz ungleich 0 und
**N−1 Abweichungen** → rot". **M-65, HEAD-Zeile 571:** Vorbedingung „der Render
**im Auslieferungszustand** endet ABWEICHUNG (nicht KETTE, nicht Format …)" →
`\Nakama\rueckweg`.

**Status: BESTÄTIGT**, mit drei Präzisierungen, die den Befund verschärfen.

1. **Es gibt keine Zeile und keinen Test für die Clip-Voraussetzung.** Die
   Nulltestzeilen sind M-60 (Renderablauf), M-61 (Format), M-62 (Identität),
   M-63 (KETTE), M-64 (Verarbeitung ein / ohne Slots), M-65 (Rückweg). Keine
   nennt „Clip ab Songbeginn". §5.7 Punkt 3 (HEAD-Zeile 790) listet für
   `nulltest.py`: „Formatprüfung, Versatzsuche, Vergleich, Urteil BITIDENTISCH,
   KETTE oder ABWEICHUNG, `ergebnis.json`, Stichprobenspitzen je Kanal,
   `--selbsttest` mit synthetischen WAV-Paaren (M-61 bis M-63)" — kein
   Messschritt für die Clip-Lage. F-17 sagt eine Messung zu, die die Matrix
   nicht trägt.
2. **Die Urteilsliste kennt „Versatz ≠ 0" nicht.** BITIDENTISCH verlangt
   Versatz 0; KETTE sagt zum Versatz nichts; ABWEICHUNG ist „sonst". Ein
   verschobener Render fällt damit zwingend in ABWEICHUNG → Exit 4 → M-65 →
   Rückweg. Zusätzlich ist die Suchbreite zu schmal: ±4 410 Frames sind
   **±0,1 s**. Ein Clip, der einen Takt später beginnt (bei den gemessenen
   140 BPM des Projekts 1,714 s, §0.5 Punkt 1, HEAD-Zeile 234), liegt weit
   außerhalb; der Versatz würde nicht einmal gefunden, und der Vergleich bei
   Versatz 0 ergäbe nahezu N Abweichungen — ununterscheidbar von einem groben
   Produktfehler.
3. **Am Basis-SHA gibt es keine Schnittstelle, die die Clip-Lage messen
   könnte.** Das Controller-Skript kennt keine Playlist-Aktion: `dispatch_command`
   führt `system.ping`, neun `transport.*`, sechzehn `mixer.*`, achtzehn
   `channels.*` und zehn `plugins.*` (`fl_controller/device_FLStudioMCP.py:175-291`,
   MCP-Repo) — kein `playlist.*`, kein Clip-Offset. `pyflp` liest das
   FL-2026-Format nicht (`eq-copilot/fixtures/fl/LIES-MICH.md:31-36`), und das
   Fixture-Blatt nennt zum Clip nur „Channel 1 `Testtrack` — Audioclip
   `Samples\Testtrack.wav` … Ziel Mixerspur 1" (`:13`), keine Startposition. Die
   Lage des Clips ist heute **nur** aus dem Render selbst erschließbar — also
   aus genau der Größe, deren Klassifikation zur Debatte steht.

Damit ist der Kern des Urteils belegt: eine verletzte Testvoraussetzung wird
dem installierten Bau als Grundgesetzbruch zugeschrieben. Dass der Rückweg
heute ohnehin abbräche (Befund 2), mildert das nicht, sondern verdeckt es: der
Lauf endete `VERFEHLT` mit „Rückweg verweigert", und die Ursache stünde nirgends.

**Einordnung (Vorschlag): DEFEKT.** F-17 macht eine prüfbare Zusage („werden
gemessen … Exit 3"), für die es weder Matrixzeile noch Test noch Bauplanschritt
gibt, und die einzige vorhandene Klassifikation führt den Fall in ABWEICHUNG
(Exit 4, Rückweg) — gegen T-12, das den automatischen Rückweg ausdrücklich nur
für „Grundgesetz im echten Host gebrochen" rechtfertigt. Der Anteil „Matrix
schweigt zur Messung der Clip-Lage" ist eine LÜCKE innerhalb desselben Defekts
und braucht in derselben Runde eine Regel.

**Schließende Änderung (Vorschlag):** (a) F-19 bekommt ein viertes Urteil
**VERSATZ** mit dem gemessenen Wert: jeder Versatz ungleich 0 ist
Szenario-Voraussetzung (Exitcode wie Befund 3, nie Exit 4, nie Rückweg), und
die Suchbreite wächst auf die volle Songlänge (Kreuzkorrelation oder Suche über
den gesamten Bereich), damit ein Taktversatz gefunden statt als Rauschen
gedeutet wird. (b) Eine neue Matrixzeile trägt diese Zusage mit Test
(`nulltest.py --selbsttest`, Attrappenpaar mit Versatz 1 und mit Versatz
75 000) und Rotbeweis „Versatzurteil entfernt → verschobene Attrappe endet
ABWEICHUNG → rot". (c) M-62 Rotbeweis (b) wird umgeschrieben: „Versatz 1 →
Urteil VERSATZ mit Wert 1, kein ABWEICHUNG". (d) M-65 nennt VERSATZ
ausdrücklich in der Liste der Fälle ohne Rückweg (neben KETTE und Format).

---

## Befund 10 [P2] — „Prüfe den Fristvertrag im tatsächlichen Szenariopfad"

**Wörtlich (Urteil):** `[P2] … NAK-286.md:495-495 — DEFEKT M-20: Ein
Szenarioschritt ohne `frist_s` übergibt am ZIEL immer explizit 4,0 Sekunden
(`tools/fl/szenario.py:123–124`). Der MCP verwendet `FL_MCP_TIMEOUT`
beziehungsweise 2,0 Sekunden dagegen nur bei `timeout is None`
(`src/fl_studio_mcp/utils/midi_connection.py:191–192`, MCP-Repo). Die
behauptete Regressionswache ist für den Szenariopfad somit bereits rot;
`test_default_timeout_from_env` prüft lediglich den Helfer. Matrix und Bauplan
müssen die fehlende Weitergabe als Baulücke erfassen und den tatsächlichen
Szenariopfad mit und ohne Umgebungswert testen.`

**Quelle an HEAD — `tools/fl/szenario.py:57-63` und `:122-124`:**

```python
def _sende(conn, aktion: str, params: dict | None, frist: float) -> dict:
    antwort = conn.send_command(aktion, params or {}, frist)
```

```python
        params = schritt.get("params") or {}
        frist = float(schritt.get("frist_s", 4.0))
        antwort = _sende(conn, aktion, params, frist)
```

**Quelle an HEAD — MCP-Repo
`src/fl_studio_mcp/utils/midi_connection.py:191-192`:**

```python
        if timeout is None:
            timeout = default_timeout()
```

Der Helfer selbst (`:54-63`) liest `FL_MCP_TIMEOUT` korrekt und fällt auf
`DEFAULT_TIMEOUT = 2.0` zurück. Der Test (`tests/test_port_selection.py:68-83`)
ruft ausschließlich diesen Helfer:

```python
def test_default_timeout_from_env(monkeypatch, raw, expected):
    …
    assert mc.default_timeout() == expected
```

`FL_MCP_TIMEOUT` kommt im Nakama-Repo an keiner Stelle vor
(`grep -rn "FL_MCP_TIMEOUT" tools/ docs/gesundheit/szenarien/`: kein Treffer);
`frist_s` steht dort nur in `tools/fl/szenario.py:123`.

**Gegenstand — Matrixzeile M-20, HEAD-Zeile 495:**

> Zustand: „Befehl **ohne eigene Frist**; `FL_MCP_TIMEOUT` gesetzt oder nicht;
> Szenarioschritt mit `frist_s`" · Zusage: „Frist = `frist_s` des Schritts,
> sonst `FL_MCP_TIMEOUT` (größer 0), **sonst 2,0 s** …" · Reihenfolge:
> „`midi_connection.py:54-63`; `tools/fl/szenario.py:57-63`, **`:123`**" ·
> Test: „**Regressionswache**: `test_default_timeout_from_env`
> (parametrisiert); die Wiederholung hat keinen Test → künftig Selbsttest
> `timeout_genau_einmal`, A35" · Rotbeweis: „Wiederholung in einer Schleife bis
> zum Erfolg → Attrappe mit zwei Timeouts besteht → rot"

Dazu §4.12 (HEAD-Zeile 623): „Regressionswachen \| M-14, M-16, M-17, **M-20
(Frist)**" und die Definition in §4 (HEAD-Zeile 461): „**„Regressionswache"**:
heute grün und gemessen".

**Status: BESTÄTIGT.** Auf dem Szenariopfad — dem einzigen, den der Laufzeit-Arm
fährt — ist die zugesagte dreistufige Frist nicht wirksam: `szenario.py:123`
setzt für jeden Schritt ohne `frist_s` den festen Wert 4,0 und übergibt ihn
explizit, sodass `timeout is None` nie eintritt und weder `FL_MCP_TIMEOUT` noch
die 2,0 s je greifen. Die Zeile zitiert `:123` selbst als Beleg ihrer Zusage,
und genau diese Zeile bricht sie — die Zeile ist in sich widersprüchlich. Das
Etikett „Regressionswache" (= „heute grün und gemessen") ist für die
Fristhälfte falsch. Weiter trägt M-20 **zwei** Zusagen (Fristkette und
„genau einmal wiederholt"), aber nur die zweite hat einen Rotbeweis; die
Fristhälfte hat als „Rotlauf" nur das Brechen eines Tests, der die Kette gar
nicht berührt. Der Beweisweg kann die Zusage nicht tragen (§2, HEAD-Zeile 345).

Widersprochen wird dem Urteil in einem Punkt der Wortwahl: „bereits rot" ist
zutreffend für die *Zusage*, aber der bestehende Test ist nicht rot — er ist
grün und misst schlicht etwas anderes. Das ist der gefährlichere Zustand
(grüner Beleg für eine unbelegte Zusage) und sollte in der Nacharbeit so
benannt werden.

**Einordnung (Vorschlag): DEFEKT** (widersprüchliche Zeile, falsche
Baseline-Klassifikation, nicht tragfähiger Beweisweg), mit einem LÜCKEN-Anteil:
die fehlende Weitergabe der Frist ist eine Baulücke, die eine eigene Zusage
braucht.

**Schließende Änderung (Vorschlag):** M-20 in zwei Zeilen teilen.
(a) **Fristweitergabe:** `szenario.py` übergibt `None`, wenn der Schritt kein
`frist_s` trägt, damit `default_timeout()` greift; Klassifikation „heute rot
(Gegenprobe am Basis-SHA)"; Test neuer Selbsttest `frist_ohne_schrittwert`
(A35) mit gesetztem und ungesetztem `FL_MCP_TIMEOUT` gegen eine
`send_command`-Attrappe, die den empfangenen Wert protokolliert; Rotbeweis
„fester Vorgabewert 4,0 wiederhergestellt → Attrappe sieht 4,0 statt des
Umgebungswerts → rot". (b) **Wiederholung genau einmal:** wie heute, Etikett
„gebaut, Test fehlt", Rotbeweis unverändert. In §4.12 fällt M-20 aus der Zeile
„Regressionswachen"; §5.7 Punkt 2 nennt die Fristweitergabe als eigene
Änderung an `szenario.py`.

---

## Befund 13 [P2] — „Korrigiere die Basisklassifikation von M-06 und M-15"

**Wörtlich (Urteil):** `[P2] … NAK-286.md:476-476 — DEFEKT M-06/M-15: Beide
Fälle besitzen am ZIEL bereits die benötigten Schnittstellen und nachweisbare
Gegenbeispiele: `Installiere` ignoriert das Ergebnis von `pruefen` und liefert
anschließend `Ok = $true` (`tools/fl/laufzeit.ps1:178–180`); `choose_port`
liefert bei mehreren passenden Ports den ersten Treffer
(`src/fl_studio_mcp/utils/midi_connection.py:47–50`, MCP-Repo). Die zugesagten
Ergebnisse sind damit heute rot, nicht „heute nicht messbar". Die Matrix muss
diese Baseline-Gegenproben entsprechend ausweisen, statt einen Rotnachweis erst
nach Implementierung vorzusehen.`

**Quelle an HEAD — `tools/fl/laufzeit.ps1:178-180`:**

```powershell
    $p = Aufgabe 'pruefen'
    Log ("  pruefen: Exit {0}, Log: {1}" -f $p.Exit, $p.Log)
    return @{ Ok = $true; Ueberspringen = $false; Grund = "installiert; pruefen Exit $($p.Exit)" }
```

**Quelle an HEAD — MCP-Repo `src/fl_studio_mcp/utils/midi_connection.py:33-51`:**

```python
def choose_port(output_ports: list[str]) -> str | None:
    """Pick the MIDI output port that leads to FL Studio, or None.

    ``FL_MCP_MIDI_PORT`` (substring, case-insensitive) wins when set; otherwise
    the first port whose name contains a known virtual-port hint is used.
    …
    for name in output_ports:
        lowered = name.lower()
        if any(hint.lower() in lowered for hint in PORT_HINTS):
            return name
    return None
```

**Gegenstand.** M-06 (HEAD-Zeile 476), Spalte Test: „**heute nicht messbar** —
der Runner wertet `pruefen` nicht (`:178-180`, `Ok = $true` bei jedem Exit);
künftig Selbsttest `pruefen_nicht_aktuell_exit3`, A35"; Rotbeweis: „**nach dem
Bau**: Wertung des `pruefen`-Exits entfernt …". M-15 (HEAD-Zeile 490): „**heute
nicht messbar** (der erste Treffer gewinnt still, `midi_connection.py:47-50`);
künftig neuer Test `test_ambiguous_virtual_ports_are_not_connected` in
`tests/test_port_selection.py`"; Rotbeweis: „**nach dem Bau**:
Mehrdeutigkeitsprüfung entfernt …".

Die Etikettendefinition steht in §4 (HEAD-Zeile 461) und kennt nur drei Werte:
„**„heute nicht messbar"**: der Mechanismus entsteht erst im Bau …
**„gebaut, Test fehlt"** … **„Regressionswache"**: heute grün und gemessen".
Ein vierter Wert wird aber an zwei anderen Stellen benutzt — §4.12
(HEAD-Zeile 625): „**heute rot (Rotlauf am Basis-SHA)** \| M-73, M-74" und §5.1
(HEAD-Zeile 659): „Für die Zeilen „heute rot" (M-73, M-74) ist der Rotlauf der
Befehl am Basis-SHA." Die Auffangregel §4.12 (HEAD-Zeile 626) lautet: „heute
nicht messbar \| **alle übrigen**".

**Status: BESTÄTIGT**, für M-15 uneingeschränkt, für M-06 mit einer
Präzisierung.

- **M-15: eindeutig falsch klassifiziert.** `choose_port` ist eine reine
  Funktion, `tests/test_port_selection.py` existiert mit sieben Testfunktionen,
  pytest läuft im MCP-Repo. Der Test
  `test_ambiguous_virtual_ports_are_not_connected` ließe sich **heute** gegen
  die unveränderte Quelle schreiben und wäre sofort rot — es muss nichts gebaut
  werden. Die Kategorie „Mechanismus entsteht erst im Bau" trifft nicht zu; es
  ist eine „heute rot"-Zeile mit Rotlauf am Basis-SHA.
- **M-06: das Gegenbeispiel steht heute fest, der Rotlauf braucht A35.** Der
  Zähler ist zitierbar (`Ok = $true` unabhängig von `$p.Exit`), und das
  Bestandskapitel des Manifests nennt ihn selbst so — §0.1, HEAD-Zeile 60:
  „Urteil „aktuell" \| **Lücke:** `pruefen` läuft, sein Exit wird nicht
  gewertet (`Ok = $true` bei jedem Exit)". Ein *ausgeführter* Rotlauf setzt
  allerdings `-Selbsttest`, `selbsttest.py` und Bein A35 voraus, die es am
  Basis-SHA nicht gibt; „Rotlauf = der Befehl am Basis-SHA" im Sinn von §5.1
  ist hier also nicht wörtlich verfügbar. Richtig ist trotzdem: „heute nicht
  messbar" ist falsch, weil §0.1 dieselbe Sache als bestehende Lücke führt —
  das Manifest widerspricht sich zwischen §0.1 und M-06.
- Dieselbe Selbstwidersprüchlichkeit gilt für M-15 gegen §0.2 (HEAD-Zeile 86):
  „**mehrere** passende Ports → der erste gewinnt still".

**Einordnung (Vorschlag): DEFEKT.** Zwei Matrixzeilen widersprechen dem
Bestandskapitel desselben Manifests, und die Klassifikation verschenkt einen
Rotbeweis, den §5.1 für genau diese Lage vorsieht. Die Ursache ist strukturell
(siehe unten), nicht nur ein Etikettenfehler an zwei Zeilen.

**Schließende Änderung (Vorschlag):** (1) Die Etikettenliste in §4
(HEAD-Zeile 461) wird um den vierten, bereits verwendeten Wert ergänzt: „**„heute
rot"**: das Verhalten ist am Basis-SHA nachweislich gegen die Zusage; der
Rotlauf ist die Gegenprobe am Basis-SHA (§5.1)" — mit dem Zusatz, dass eine
Gegenprobe, deren Bein erst im Bau entsteht, als „heute rot (Gegenprobe
belegt, Rotlauf mit A35)" geführt wird. (2) M-15 wechselt auf „heute rot",
Rotlauf = der neue Test gegen die unveränderte `choose_port` am Basis-SHA, mit
Rohausgabe. (3) M-06 wechselt auf „heute rot (Gegenprobe `:178-180`, Rotlauf
mit A35)" und verweist auf §0.1 Zeile 60. (4) Die Auffangzeile §4.12
(HEAD-Zeile 626) „alle übrigen" wird aufgelöst: jede Zeile trägt ihr Etikett
ausdrücklich, damit der Auffang nicht erneut falsche Baselines erzeugt. (5) Der
Bauer prüft in derselben Runde die übrigen als „heute nicht messbar"
klassifizierten Zeilen gegen §0.1/§0.2 auf denselben Fehler.

---

## Gemeinsame Ursachen

**Vermutung des Dirigenten „Reihenfolge FL↔Rückweg↔Exitcodes hinter 2, 3, 4":
bestätigt, und schärfer fassbar.** Die drei Befunde sind ein Fehler in zwei
Ausprägungen:

1. **Der Exitcode ist ein einziger Kanal für drei verschiedene Aussagen.**
   „Diesem Szenario fehlt eine Voraussetzung" (KETTE M-63, Format M-61, Clip
   F-17), „dem Lauf fehlt eine Voraussetzung, Weitermachen ist sinnlos" (kein
   Port, kein Ping) und „der installierte Bau ist schuldig" (ABWEICHUNG →
   Rückweg) teilen sich 3 und 4. Deshalb bricht eine Kettenmessung die
   Folgeszenarien ab (Befund 3), und deshalb landet eine verletzte
   Testvoraussetzung im Schuldkanal (Befund 4).
2. **Die Matrix modelliert den Nulltest als reinen Dateivergleich und nicht als
   Zustandsmaschine des Runners.** Wer wann läuft (FL oben/unten), ist in keiner
   Zeile Vorbedingung. Deshalb steht in M-65 eine Aktion, deren Voraussetzung
   der Ablauf desselben Runners zuverlässig zerstört (Befund 2), und deshalb
   fehlt in Befund 3 die Antwort, was nach einem Rückweg mit den restlichen
   Szenarien geschieht.

Die drei Befunde gehören deshalb in **einen** Nacharbeitsauftrag: Exitcode- und
Urteilskanäle trennen (additiv nach R-286-5), die Reihenfolge FL beenden →
Rückweg → Prüfen festschreiben, und die Nulltest-Zeilen um die Zustände
„VERSATZ" und „FL läuft/beendet" ergänzen.

**Vermutung „Klassifikation heute rot hinter 10 und 13": bestätigt, mit
benennbarer Wurzel.** Die Etikettenliste in §4 (HEAD-Zeile 461) definiert drei
Werte, §4.12 und §5.1 benutzen aber vier („heute rot"). Zusammen mit der
Auffangregel „heute nicht messbar \| alle übrigen" (HEAD-Zeile 626) rutscht
jede Zeile, deren Zusage heute nachweislich verletzt ist, in die falsche, zu
milde Kategorie (M-06, M-15) — oder, wenn sie zufällig einen grünen Test in der
Nähe hat, sogar in die zu starke („Regressionswache" für M-20). Ein Auftrag,
der nur die drei Zeilen korrigiert, ohne die Etikettenliste und den Auffang zu
reparieren, erzeugt denselben Fehler wieder. Die Reparatur der Liste gehört
deshalb in denselben Änderungssatz.

---

## Schlusstabelle Teil B

| Befund | Status | Einordnung (Vorschlag) | Schließende Änderung (Kurz) |
|---|---|---|---|
| 2 [P1] Rückweg vor FL-Ende (M-65/§5.7) | BESTÄTIGT | DEFEKT | Reihenfolge „Diagnose-FL beenden → `\Nakama\rueckweg` → `\Nakama\pruefen`" in M-65 und §5.7 festschreiben; Selbsttest mit lebender FL-Attrappe rot; Kopfzeile trennt „verweigert (FL läuft)" von „(NAK-41)"; Umgang mit den Restszenarien nach dem Rückweg klären |
| 3 [P2] KETTE bricht Folgeszenarien (M-63/M-66/F-26 gegen M-10) | BESTÄTIGT | DEFEKT | Szenario-Voraussetzung von Lauf-Voraussetzung trennen: neuer additiver Exitcode, Gesamturteil bleibt 3, Schleife läuft weiter; M-10, M-63, M-66 bis M-68 nachziehen; Selbsttest `kette_erreicht_folgeszenarien` |
| 4 [P2] Clip-Voraussetzung gegen Audioabweichung (F-17/F-19/M-62) | BESTÄTIGT | DEFEKT (mit LÜCKEN-Anteil: Messung der Clip-Lage) | Viertes Urteil VERSATZ mit gemessenem Wert und Suche über die volle Songlänge; neue Matrixzeile mit Test und Rotbeweis; M-62 Rotbeweis (b) umschreiben; M-65 nennt VERSATZ als Fall ohne Rückweg |
| 10 [P2] Fristvertrag im Szenariopfad (M-20) | BESTÄTIGT | DEFEKT (mit LÜCKEN-Anteil: fehlende Weitergabe) | M-20 teilen: (a) `szenario.py` übergibt `None` ohne `frist_s`, Etikett „heute rot", neuer Selbsttest mit und ohne `FL_MCP_TIMEOUT`; (b) Wiederholung wie heute; M-20 fällt aus §4.12 „Regressionswachen" |
| 13 [P2] Basisklassifikation M-06/M-15 | BESTÄTIGT (M-15 uneingeschränkt; M-06 mit Präzisierung: Gegenprobe heute belegt, Rotlauf erst mit A35) | DEFEKT | Etikettenliste §4 um „heute rot" ergänzen; M-15 → „heute rot" (Rotlauf am Basis-SHA), M-06 → „heute rot (Gegenprobe `:178-180`, Rotlauf mit A35)"; Auffangregel §4.12 „alle übrigen" auflösen; übrige „nicht messbar"-Zeilen gegen §0.1/§0.2 nachprüfen |
