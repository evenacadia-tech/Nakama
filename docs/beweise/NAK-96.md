# Beweismanifest — NAK-96 «Runner-Rohausgaben trennen, git ohne Index-Lock»

| Feld | Wert |
|---|---|
| Ticket | `NAK-96` (Punkte 1 und 2; Punkt 3 war beim Auftrag bereits im Skill erledigt) |
| Quelle | `docs/offene-punkte.md`, Zeile `| NAK-96 |` — **wörtlich**, nicht meine Zusammenfassung |
| Basis-SHA | `2271df5bc7ab2ba823ecbf1f7c7dbb68e16984d8` (Zweig `master`) |
| Datum | 2026-08-29 |
| Werkzeugeingriff | `tools/beweise.ps1`, `tools/dirigent/cockpit.ps1`, `docs/beweise/VORLAGE.md`, `CLAUDE.md` |

---

## 1. Auftrag, wörtlich aus `docs/offene-punkte.md`

> **Runner und Cockpit: Kanon-Rohausgaben raus aus den Manifesten, `git status`
> ohne Index-Lock.** (1) `tools/beweise.ps1 -Anhaengen` hängt je Lauf ~3000
> Zeilen Rohausgabe an das Manifest; `docs/beweise/SONDE-007b.md` hat 24 000,
> `SONDE-010.md` 16 000 Zeilen — für Prüfer und Dirigent unlesbar, Kontext geht
> an Logs verloren. Ziel: Rohausgabe nach `docs/beweise/roh/<TICKET>-<sha>.md`,
> im Manifest nur die Kopfzeile (`**Lauf:** …`) mit Verweis; Beglaubigungslogik
> unverändert; bestehende Manifeste bleiben, wie sie sind. (2)
> `tools/dirigent/cockpit.ps1` ruft `git status`/`git rev-parse` mit 4–5 s
> Timeout; wird `git status` beim Index-Refresh während eines Baus abgeschossen,
> bleibt eine 0-Byte `.git/index.lock` — am 29.08. dreimal aufgetreten, jeweils
> unter Last. Fix: alle Cockpit-Git-Aufrufe mit `--no-optional-locks` (bzw.
> `GIT_OPTIONAL_LOCKS=0`), und der Runner mit demselben Schalter, wo er nur
> liest.

Nicht im Auftrag und **nicht** angefasst: NAK-93 (Broker-Bau im Kanon), NAK-94
(A17-Hashvergleich), bestehende Manifeste, Legacy-Umbenennungen, andere Beine
des Runners.

---

## 2. Umgesetzte Entscheidungen (A–E)

| # | Entscheidung | Ort im Code |
|---|---|---|
| A | Aufteilung je Lauf: Manifest bekommt Überschrift, unveränderte `**Lauf:** …`-Zeile plus angehängten ` \| **Rohausgabe:** …`-Verweis, `### Kopf - woran gemessen wurde`, den VERALTET-Hinweis falls zutreffend und `### Uebersicht`; die Roh-Datei bekommt eigenen Kopf plus den vollständigen bisherigen Block (Kopf, unbestätigte Dateien, Baustand, Übersicht, Rohe Ausgaben, Bau vor dem Lauf). Beglaubigungslogik, Urteilstexte und Exitcodes unverändert. | `tools/beweise.ps1`, Abschnitt „Manifest" |
| B | Roh-Dateiname `docs/beweise/roh/<TICKET>-<sha7>.md`; schmutziger Arbeitsbaum → Suffix `-dirty`; existierende Datei → `-2`, `-3`, …; nie überschreiben; Verzeichnis bei Bedarf angelegt. | `tools/beweise.ps1`, Block „Rohausgabe-Datei bestimmen" |
| C | Ohne `-Anhaengen` (frisches Ziel) gilt dieselbe Aufteilung. | derselbe Block, `$Anhaengen` steuert nur Überschrift und Schreibmodus des Manifests |
| D | `--no-optional-locks` als **erstes** Argument vor `-C` in `Invoke-GitText` (Cockpit), `Git-Wert` (Runner) und im JUCE-`describe`-Aufruf. | `tools/dirigent/cockpit.ps1`, `tools/beweise.ps1` |
| E | Doku nachgezogen: Kommentarkopf und `.PARAMETER` im Runner, `docs/beweise/VORLAGE.md` §3, `CLAUDE.md` „Bauen und beweisen"; `.gitignore` schließt `docs/beweise/roh/` nicht aus. | siehe §4 |

### Eigene technische Entscheidungen innerhalb der Grenze

1. **Relativer Verweis wird gerechnet, nicht zusammengeklebt.** Statt `roh/<datei>.md`
   fest zu verdrahten, rechnet der Runner `[IO.Path]::GetRelativePath` vom
   Manifest-Verzeichnis zur Roh-Datei und zurück. Im Normalfall (Manifest direkt
   in `docs/beweise/`) ergibt das exakt den vorgegebenen Text `roh/<datei>.md`;
   liegt ein Manifest in einem Unterordner, zeigt der Verweis trotzdem richtig.
   Die Bruchprobe in §6 fährt genau diesen Fall.
2. **Roh-Datei wird vor dem Manifest geschrieben.** Sonst verweist ein bereits
   geschriebenes Manifest auf eine Datei, die nie entstanden ist, falls das
   zweite Schreiben scheitert.
3. **Listenvariable heißt `$roh`, nicht `$r`.** `$r` ist im Skript-Scope des
   Runners schon für Prozessergebnisse belegt (Zeile 545); unter
   `Set-StrictMode -Version Latest` wäre die Doppelbelegung zwar zulässig, aber
   eine Lesefalle.
4. **Commit-Zuschnitt.** Der Auftrag schlug vier Commits vor, davon zwei
   innerhalb derselben Datei (`tools/beweise.ps1`: Aufteilung und
   `--no-optional-locks`). Ein hunkweiser Split ginge nur interaktiv
   (`git add -p`), was in dieser Umgebung nicht verfügbar ist. Zuschnitt deshalb
   nach Datei: (1) Runner, (2) Cockpit, (3) Doku + Manifest, (4) Kanon-Anhang
   und Roh-Datei.

---

## 3. Prüfliste `tools/dirigent/pruefliste.md` — wo gemessen

| Klasse | Zutreffend? | Wo gemessen / warum nicht |
|---|---|---|
| **A** Rückstau und Prioritätsklassen | nein | Der Eingriff berührt keine IPC-Queue, keinen Puffer und keine Prioritätsklasse; geändert sind nur Dateiausgabe und git-Aufrufe. |
| **B** Lebenszyklus | nein | Kein Verbinden/Trennen, kein Start/Stop, kein Thread. Der einzige Paarcharakter des Eingriffs ist schreiben↔verweisen und steht unter F. |
| **C** Verträge und Längen | teilweise | Der einzige Vertrag im Eingriff ist der Wortlaut der Lauf-Zeile, aus dem `tools/plan/planstand.py` (Regex `KANON`, Zeile 66) die Kanon-Zahl zurückliest. Gemessen in **§5.1** mit genau dieser Regex gegen das erzeugte Manifest. |
| **D** Bau- und Prüfriegel | ja | Beglaubigungslogik (`$veraltet`, Exit 4) ist unverändert — nachgewiesen in **§5.5**: der Abschlusslauf beglaubigt weiterhin und trägt denselben Urteilstext. Der Runner bezeugt weiterhin nur, was er gebaut hat; NAK-93/NAK-94 bleiben offen und wurden nicht angefasst. |
| **E** Behauptung ≤ Messung | ja | Jede Behauptung in §5 steht neben ihrer Rohausgabe. Die Kollisionsprobe wurde einmal absichtlich gebrochen (**§6**), Rohausgabe des Rots liegt bei. Über den Index-Lock wird **nicht** behauptet „kein `index.lock` mehr" — das ist nicht messbar; behauptet und gemessen wird nur „jeder lesende git-Aufruf in Cockpit und Runner trägt den Schalter" (**§5.4**). |
| **F** Änderungssatz | ja | schreiben↔verweisen im selben Commit: derselbe Codeblock erzeugt die Roh-Datei und den Verweis darauf, und beide Richtungen (Manifest→Roh, Roh→Manifest) entstehen aus denselben zwei gerechneten Pfaden. Doku (Runner-Kopf, VORLAGE §3, CLAUDE.md) liegt im selben Änderungssatz wie das Verhalten, das sie beschreibt. |

---

## 4. Diff-Übersicht

**Befehl:** `git --no-optional-locks diff --stat -- CLAUDE.md docs/beweise/VORLAGE.md tools/beweise.ps1 tools/dirigent/cockpit.ps1` · **Datum:** 2026-08-29

```text
 CLAUDE.md                  |   4 +-
 docs/beweise/VORLAGE.md    |  15 +++
 tools/beweise.ps1          | 235 +++++++++++++++++++++++++++++++++------------
 tools/dirigent/cockpit.ps1 |   9 +-
 4 files changed, 200 insertions(+), 63 deletions(-)
```

Die 235 Zeilen in `tools/beweise.ps1` sind überwiegend Verschiebung: der bisher
einteilige Ausgabeblock ist in zwei Listen (`$z` Manifest, `$roh` Rohausgabe)
zerlegt; neu sind der Block „Rohausgabe-Datei bestimmen", der Kopf der
Roh-Datei und der Kommentarkopf.

---

## 5. Proben

Die am Ende dieser Datei angehängten `## Kanon-Lauf`-Abschnitte stammen aus den
Probeläufen: **Kollisionsprobe Lauf 1 und 2** (§5.2, ohne `-Bauen`, deshalb
`VERALTET` und Exit 2) und der **Abschlusslauf** (§5.5, mit `-Bauen`). Sie
stehen absichtlich hier: der Umbau lässt sich nur an echten Läufen messen, und
ihre Roh-Dateien liegen unter `docs/beweise/roh/`.

<a id="b51"></a>
### 5.1 · Regex-Probe: die Kanon-Zahl bleibt im Manifest lesbar

`tools/plan/planstand.py` liest die Kanon-Zahl mit `KANON` (Zeile 66) aus der
Lauf-Zeile. Wandert diese Zeile mit der Rohausgabe aus dem Manifest, verliert
der Planstand die Zahl still — deshalb ist sie im Manifest geblieben und der
Verweis nur angehängt worden.

_(Messung nach dem Abschlusslauf — §5.5 — eingetragen: siehe unten §5.1a.)_

<a id="b52"></a>
### 5.2 · Kollisionsprobe: zwei Läufe auf demselben HEAD

Beide Läufe auf `2271df5`, Arbeitsbaum unbestätigt (deshalb `-dirty` im Namen),
ohne `-Bauen`.

**Befehle:**

```powershell
pwsh -NoProfile -File tools/beweise.ps1 -Ziel docs/beweise/NAK-96.md -Anhaengen -Titel 'NAK-96 Kollisionsprobe Lauf 1 (ohne -Bauen)'
pwsh -NoProfile -File tools/beweise.ps1 -Ziel docs/beweise/NAK-96.md -Anhaengen -Titel 'NAK-96 Kollisionsprobe Lauf 2 (ohne -Bauen)'
```

```text
=== Kollisionsprobe Lauf 1 (Suffixlogik AN) ===

ROT - 1 von 32 Kanon-Laeufen fehlgeschlagen | 2 geplante Pruefung(en) noch nicht gebaut | 1 stillgelegte(s) Bein(e), siehe Uebersicht
Manifest:   docs\beweise\NAK-96.md
Rohausgabe: docs\beweise\roh\NAK-96-2271df5-dirty.md
exit=2
SHA-256 roh/NAK-96-2271df5-dirty.md nach Lauf 1: 085CE7BCB9608579ACEB6888FB5D58A94DA7CF01B7C199D188B130CA3A481C3C

=== Kollisionsprobe Lauf 2 (gleicher HEAD, gleicher Arbeitsbaum-Zustand) ===

ROT - 1 von 32 Kanon-Laeufen fehlgeschlagen | 2 geplante Pruefung(en) noch nicht gebaut | 1 stillgelegte(s) Bein(e), siehe Uebersicht
Manifest:   docs\beweise\NAK-96.md
Rohausgabe: docs\beweise\roh\NAK-96-2271df5-dirty-2.md
exit=2
SHA-256 roh/NAK-96-2271df5-dirty.md nach Lauf 2: 085CE7BCB9608579ACEB6888FB5D58A94DA7CF01B7C199D188B130CA3A481C3C
Erste Rohausgabe unveraendert: True

=== Verzeichnis docs/beweise/roh ===

Name                               Length
----                               ------
NAK-96-2271df5-dirty-2.md          189815
NAK-96-2271df5-dirty.md            189820
NAK-96-bruchprobe-2271df5-dirty.md 189829
NAK-96-bruchprobe.md                37715
```

**Gemessen:** der zweite Lauf legt `…-dirty-2.md` an; die Roh-Datei des ersten
Laufs trägt vor und nach dem zweiten Lauf denselben SHA-256
(`085CE7BC…481C3C`) — sie wurde nicht überschrieben. Ohne die Suffixlogik ist
genau das nicht der Fall (§6).

**Das rote Bein ist nicht NAK-96:** B1 `EqCopIdentityTest`, Exit 1, „114
Pruefungen ok, 2 Fehler — moduleinfo.json ist nicht aelter als
plugin/CMakeLists.txt [29 Aug 2026 12:52:08pm vs 29 Aug 2026 1:30:46pm]". Das
ist der Frischeriegel des Beins gegen einen Lauf **ohne** `-Bauen`: die
generierte `moduleinfo.json` stammt vom letzten Bau, `CMakeLists.txt` ist
jünger. Der Abschlusslauf mit `-Bauen` (§5.5) erzeugt sie neu.

<a id="b53"></a>
### 5.3 · Umfangsprobe

_(Messung nach dem Abschlusslauf — §5.5 — eingetragen: siehe unten §5.3a.)_

<a id="b54"></a>
### 5.4 · Cockpit und der git-Schalter

**Trockentest des Schalters** auf Git 2.54.0.windows.1:

```text
=== git --no-optional-locks Trockentest ===
 M CLAUDE.md
 M docs/beweise/VORLAGE.md
 M tools/beweise.ps1
 M tools/dirigent/cockpit.ps1
?? docs/beweise/NAK-96.md
exit=0
git version 2.54.0.windows.1
```

**Cockpit fährt.** `-StatusLine` mit JSON auf stdin:

```text
=== StatusLine ===
NAKAMA · Fable 5/ · 14/38 fertig · Dirigent über /dirigent

exit=0
```

`-Plan`:

```text
=== cockpit -Plan ===
! PLANQUELLEN NICHT FRISCH · erst tools/plan/planstand.py ausführen
exit=4
```

Exit 4 ist hier die **richtige** Antwort des Cockpits, kein Fehler: unter
`docs/beweise/` liegen zu diesem Zeitpunkt unbestätigte Dateien (dieses
Manifest und die Roh-Dateien), also sind die Planquellen gegenüber dem letzten
Commit nicht frisch. Genau diese Aussage kommt aus dem `git status --porcelain`,
der jetzt mit dem Schalter läuft. Der Lauf nach den Commits steht in §7.

**Beleg, dass jeder lesende git-Aufruf den Schalter trägt.** Beide Dateien
starten git nur an zwei Stellen; alle übrigen Aufrufe gehen durch diese zwei
Hüllen (`Invoke-GitText` im Cockpit, `Git-Wert` im Runner) plus den einen
JUCE-`describe`:

```text
=== Jeder git-Prozessaufruf in Cockpit und Runner ===
tools/dirigent/cockpit.ps1:179:        $gitPath = (Get-Command git -ErrorAction Stop).Source
tools/dirigent/cockpit.ps1:180:        return Invoke-TextProcess $gitPath "--no-optional-locks -C $(Quote-ProcessArgument $script:RepoRoot) $Arguments" $TimeoutSeconds
tools/beweise.ps1:182:    $r = Fuehre-Aus -Datei 'git' -Argumente (@('--no-optional-locks', '-C', $Wurzel) + $Argumente)
tools/beweise.ps1:575:    $r = Fuehre-Aus -Datei 'git' -Argumente @('--no-optional-locks', '-C', $juceQuelle, 'describe', '--tags', '--always', '--dirty')

=== Gegenprobe: git-Prozessaufrufe OHNE --no-optional-locks ? ===
(keine Zeile zwischen den Markern = jeder Prozessaufruf traegt den Schalter)

=== alle Aufrufer von Invoke-GitText (Cockpit) ===
169:function Invoke-GitText {
236:        $source = Invoke-GitText 'log -1 --format=%h -- docs/plan docs/beweise tools/plan'
238:        $dirtySources = Invoke-GitText 'status --porcelain -- docs/plan docs/beweise tools/plan'
241:        $dirtyTree = Invoke-GitText 'status --porcelain'
244:        $worktrees = Invoke-GitText 'worktree list --porcelain'
663:        $headRead = Invoke-GitText 'rev-parse HEAD'
854:    $baseCheck = Invoke-GitText "merge-base --is-ancestor $BaseSha HEAD"
887:        $head = Invoke-GitText 'rev-parse HEAD'
892:                $baseCheck = Invoke-GitText "merge-base --is-ancestor $BaseSha HEAD"
900:        $worktrees = Invoke-GitText 'worktree list --porcelain'

=== alle Aufrufer von Git-Wert (Runner) ===
175:function Git-Wert {
591:$schmutzig = Git-Wert @('status', '--porcelain')
600:    'Zweig'           = (Git-Wert @('rev-parse', '--abbrev-ref', 'HEAD'))
601:    'Commit'          = (Git-Wert @('log', '-1', '--format=%h %s'))
602:    'Commit (voll)'   = (Git-Wert @('rev-parse', 'HEAD'))
```

Alle neun Cockpit-Aufrufe (`log`, `status`, `worktree list`, `rev-parse`,
`merge-base --is-ancestor`) und alle vier Runner-Aufrufe (`status`, `rev-parse`
×2, `log`) sind lesend; keiner schreibt.

> **Grenze der Behauptung (Prüfliste E):** hier steht **nicht** „es entsteht
> kein `.git/index.lock` mehr" — das lässt sich in dieser Session nicht messen,
> weil der Fehler nur unter Baulast und mit abgeschossenem Prozess auftrat.
> Gemessen und behauptet ist: *jeder lesende git-Aufruf in Cockpit und Runner
> trägt `--no-optional-locks`*, und der Schalter ist auf der installierten
> git-Version 2.54.0.windows.1 gültig (Exit 0 im Trockentest oben).

<a id="b55"></a>
### 5.5 · Abschlusslauf: voller Kanon auf dem committeten Endstand

_(wird nach den Commits eingetragen — siehe unten §5.5a.)_

---

## 6. Bruchprobe (Prüfliste E): Suffixlogik absichtlich ausgeschaltet

Gebrochen wurde die Kollisionsprobe aus §5.2. Dazu ist in
`tools/beweise.ps1` die Schleife, die einen freien Dateinamen sucht,
auskommentiert worden:

```powershell
$rohZaehler = 2
# BRUCHPROBE NAK-96 - absichtlich ausgeschaltet, wird zurueckgenommen:
# while (Test-Path -LiteralPath $rohDatei) {
#     $rohDatei = Join-Path $rohVerzeichnis ('{0}-{1}.md' -f $rohBasis, $rohZaehler)
#     $rohZaehler++
# }
```

Zwei Läufe auf denselben Stand, eigenes Ziel `docs/beweise/roh/NAK-96-bruchprobe.md`
(bewusst **im** roh-Verzeichnis, damit dieser Lauf die Belege aus §5.2 nicht
anfasst und zugleich der Sonderfall „Manifest liegt nicht in `docs/beweise/`"
den gerechneten Relativverweis mitprüft):

```text
=== Bruchprobe Lauf 1 (Suffixlogik AUS) ===
ROT - 1 von 32 Kanon-Laeufen fehlgeschlagen | 2 geplante Pruefung(en) noch nicht gebaut | 1 stillgelegte(s) Bein(e), siehe Uebersicht
Manifest:   docs\beweise\roh\NAK-96-bruchprobe.md
Rohausgabe: docs\beweise\roh\NAK-96-bruchprobe-2271df5-dirty.md
exit=2
Dauer: 129,6 s
--- SHA-256 nach Lauf 1 ---
Hash : 7CD269903218FB7BBAD6EC493260031C702A3A57F0E4C8B9C7583B3E4BE34E22
Path : C:\Users\phili\Projekte\Nakama\docs\beweise\roh\NAK-96-bruchprobe-2271df5-dirty.md

=== Bruchprobe Lauf 2 (Suffixlogik AUS) ===
ROT - 1 von 32 Kanon-Laeufen fehlgeschlagen | 2 geplante Pruefung(en) noch nicht gebaut | 1 stillgelegte(s) Bein(e), siehe Uebersicht
Manifest:   docs\beweise\roh\NAK-96-bruchprobe.md
Rohausgabe: docs\beweise\roh\NAK-96-bruchprobe-2271df5-dirty.md
exit=2
--- SHA-256 nach Lauf 2 ---
Hash : 51913D297B42D3782942A12FF716906D79E7792BC1298E4F423EA631B7211896
Path : C:\Users\phili\Projekte\Nakama\docs\beweise\roh\NAK-96-bruchprobe-2271df5-dirty.md

--- Dateien im roh-Verzeichnis ---
Name                               Length
----                               ------
NAK-96-bruchprobe-2271df5-dirty.md 189829
NAK-96-bruchprobe.md                37715
```

**Das ist das Rot:** beide Läufe schreiben in **denselben** Pfad, der SHA-256
wechselt von `7CD26990…4E22` auf `51913D29…1896`, und im Verzeichnis liegt nach
zwei Läufen nur **eine** Roh-Datei. Die Rohausgabe des ersten Laufs ist weg,
während das Manifest `NAK-96-bruchprobe.md` weiter zwei Abschnitte trägt, die
beide auf diese eine Datei zeigen — der zweite Abschnitt verweist auf Zahlen,
die er nicht erzeugt hat.

Der Bruch ist unmittelbar danach zurückgenommen worden; §5.2 ist mit der
zurückgenommenen Fassung gefahren und ist grün. Der Vergleich ist damit
diskriminierend: dieselbe Probe, einmal ohne und einmal mit dem Fix, mit
unterschiedlichem Ergebnis.

Die beiden Bruchprobe-Dateien bleiben liegen, statt gelöscht zu werden — sie
sind der Beleg. `NAK-96-bruchprobe.md` ist zugleich das einzige Beispiel im
Repo für ein Manifest, das selbst im roh-Verzeichnis liegt; sein
Rohausgabe-Verweis lautet dort korrekt `NAK-96-bruchprobe-2271df5-dirty.md`
ohne `roh/`-Präfix, weil er gerechnet und nicht zusammengeklebt wird.

---

## 7. Commits und Abschluss

_(wird nach den Commits eingetragen.)_

---

## Kanon-Lauf - NAK-96 Kollisionsprobe Lauf 1 (ohne -Bauen)

**Lauf:** 2026-08-29 14:00 | **Runner:** `tools/beweise.ps1` | **Urteil:** ROT - 1 von 32 Kanon-Laeufen fehlgeschlagen | 2 geplante Pruefung(en) noch nicht gebaut | 1 stillgelegte(s) Bein(e), siehe Uebersicht | **Exitcode:** 2 | **Rohausgabe:** [roh/NAK-96-2271df5-dirty.md](roh/NAK-96-2271df5-dirty.md)

### Kopf - woran gemessen wurde

| Feld | Wert |
|---|---|
| Zeitpunkt | 2026-08-29 14:00:56 +02:00 |
| Rechner | SCHUBBINATOR200 \| Windows 10.0.26200.0 |
| Zweig | master |
| Commit | 2271df5 NEXT-SESSION: Nachtrag Dirigentenrunde 29.08. (FL-Termin, Fragenrunde, SONDE-010 R3, Prozessumbau, Reihenfolge) |
| Commit (voll) | 2271df5bc7ab2ba823ecbf1f7c7dbb68e16984d8 |
| Arbeitsbaum | 7 unbestaetigte Datei(en) - dieser Lauf beweist NICHT allein den Commit |
| JUCE gepinnt | 8.0.9 |
| JUCE auf Platte | 8.0.9-dirty |
| FL Studio | FL Studio 2025 25.2.5.5319 \| FL Studio 2026 26.1.4.5589 |
| cargo | cargo 1.93.1 (083ac5135 2025-12-15) |
| rustc | rustc 1.93.1 (01f6ddf75 2026-02-11) |
| PowerShell | 7.6.5 |

> **VERALTET - dieser Lauf beweist NICHT den aktuellen Quellstand.**
> Mindestens eine Pruefbinaerdatei ist aelter als die Quellen. Neu fahren mit `-Bauen`.

### Uebersicht

| # | Behauptung | Befehl | Ergebnis | Dauer | Rohausgabe |
|---|---|---|---|---|---|
| A1 | Passthrough ist bitgleich; 0 Samples Latenz, 0 Tail; NaN/Inf werden gezaehlt, aber nicht veraendert. | `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | [OK] Exit 0 | 0,06 s | [A1](roh/NAK-96-2271df5-dirty.md#a1) |
| A2 | AnalyseEngine deckt sich mit der eingefrorenen Offline-Referenz (Fixture-SHA-256 als Determinismus-Riegel). | `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | [OK] Exit 0 | 11,59 s | [A2](roh/NAK-96-2271df5-dirty.md#a2) |
| A3 | Hoer-Markierung bleibt verriegelt: Render/Freilauf bitgleich, Analyse-Abgriff sitzt vor der Faerbung, und der U10-Term ist gedeckt - OHNE Playhead faerbt bei sonst voller Erlaubnis kein Sample (T11, mit Gegenprobe bei laufendem Transport). | `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | [OK] Exit 0 | 6,45 s | [A3](roh/NAK-96-2271df5-dirty.md#a3) |
| A4 | Broker-Vertragstests gruen (Framing, Protokoll, Bindung, Aggregat, Server). | `cargo test --manifest-path broker/Cargo.toml --color never` | [OK] Exit 0 | 3,16 s | [A4](roh/NAK-96-2271df5-dirty.md#a4) |
| A4b | C++-PipeClient: stop/reconnect und ganze Frames sind zeitlich begrenzt, SQOS ist Identification, Peerbytes sind UTF-8/NUL-sauber, ACK-Sequenzen streng und u64-Zaehler wire-sicher. | `eq-copilot\build\plugin\EqCopPipeClientTest_artefacts\Release\EqCopPipeClientTest.exe` | [OK] Exit 0 | 1,08 s | [A4b](roh/NAK-96-2271df5-dirty.md#a4b) |
| A5 | Referenzbein (jsonschema, draft 2020-12): Schema haelt die Engine-Teilmenge ein, Textriegel deckt jede gemessene Kante, jedes Fixture wird wie im Manifest klassifiziert, jede Definition hat ein Negativfixture. | `py -3.13 tools\eq-copilot\pruefe_v3_vertrag.py --abdeckung` | [OK] Exit 0 | 1,80 s | [A5](roh/NAK-96-2271df5-dirty.md#a5) |
| A6 | Beide Bandgitter sind bytegleich zur Neuerzeugung; 221 Baender, 64 Gruppen als exakte Partition. | `py -3.13 tools\eq-copilot\erzeuge_bandgitter.py --pruefen` | [OK] Exit 0 | 0,16 s | [A6](roh/NAK-96-2271df5-dirty.md#a6) |
| A7 | Quantisierungsvertrag bytegleich zur Neuerzeugung; Rundung, Saettigung und Nichtendliches als Testvektoren. | `py -3.13 tools\eq-copilot\erzeuge_quantisierung.py --pruefen` | [OK] Exit 0 | 0,16 s | [A7](roh/NAK-96-2271df5-dirty.md#a7) |
| A8 | Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_v3_fixtures.py --pruefen` | [OK] Exit 0 | 0,22 s | [A8](roh/NAK-96-2271df5-dirty.md#a8) |
| A9 | Codegen-Drift ist 0: die Neugenerierung aus dem .fbs ist bytegleich zum committeten C++- und Rust-Code; flatc, C++-Header und Rust-Crate tragen dieselbe gepinnte Version; jedes Tabellenfeld traegt eine explizite Feld-ID. | `py -3.13 tools\eq-copilot\pruefe_flatc_drift.py` | [OK] Exit 0 | 0,32 s | [A9](roh/NAK-96-2271df5-dirty.md#a9) |
| A10 | Binaerer Fixture-Korpus und sein MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_fb_fixtures.py --pruefen` | [OK] Exit 0 | 1,16 s | [A10](roh/NAK-96-2271df5-dirty.md#a10) |
| A11 | Die fuenf v2-Vertraege (ipc v2, measurement v1, report v1, snapshot v3, aggregat v1) sind gueltiges JSON und gueltige JSON-Schemas; ihre $id-Familie ist eingefroren. | `py -3.13 tools\eq-copilot\pruefe_v2_schemas.py` | [OK] Exit 0 | 0,31 s | [A11](roh/NAK-96-2271df5-dirty.md#a11) |
| A12 | Parameterbestand (109 IDs, §53.8) haelt den Vertrag; RFC-8785-Zahlenvektoren tragen den RFC-Text und werden von rfc8785 bestaetigt; State-Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung. | `py -3.13 tools\eq-copilot\erzeuge_state_fixtures.py --pruefen` | [OK] Exit 0 | 0,31 s | [A12](roh/NAK-96-2271df5-dirty.md#a12) |
| A13 | Capabilityreport FL: die zehn Bits aus §53.6 entsprechen der v3-Vertragsform und stehen so, wie die Rohdaten der Termine A und B sie tragen; jedes supported hat einen Termin, jedes unsupported seinen festen Fallback. | `py -3.13 tools\eq-copilot\pruefe_host_capabilities.py` | [OK] Exit 0 | 0,27 s | [A13](roh/NAK-96-2271df5-dirty.md#a13) |
| A14 | Der gemeinsame Kern traegt keine Bundle-Identitaet: NakamaKern.lib enthaelt keinen eingefrorenen Identitaetswert (Namen, Viercodes, CIDs roh und COM-vertauscht) und genau seine eigenen Uebersetzungseinheiten, kein JUCE-Modulobjekt; die Gegenprobe findet dieselben Werte im gebauten EQ-Copilot-Bundle. | `py -3.13 tools\eq-copilot\pruefe_kern_identitaetsfrei.py` | [OK] Exit 0 | 0,59 s | [A14](roh/NAK-96-2271df5-dirty.md#a14) |
| A17 | Installer-Manifest: keine zweite Identitaet (kein Viercode, keine Class-ID), jeder Quellpfad ist der aus Ziel + Identitaetsdatei nachgerechnete BUNDLE-ORDNER, `art` ist eine geschlossene Menge, jedes Ziel genau einmal, Broker aus dem Crate-Namen, Zielverzeichnisse geschuetzt, Signaturzeile behauptet keine Pruefung ohne Mittel, Rueckweg samt NAK-41-Riegel und hash_art vollstaendig; jede Regel faellt an einem verdorbenen Manifest; Ordner-Hash v1 bytegleich in Python und PowerShell, Nicht-ASCII bricht beide Haelften ab. | `py -3.13 tools\eq-copilot\pruefe_installer_manifest.py` | [OK] Exit 0 | 1,01 s | [A17](roh/NAK-96-2271df5-dirty.md#a17) |
| A18 | Gegenpfad installieren<->Rueckweg gefahren (Sandbox, nichts installiert): Erstinstallation traegt moduleinfo.json, ein Tausch ersetzt statt zu mischen, -Pruefen sieht den ganzen Ordner, der Rueckweg stellt den Vorzustand bytegleich her und laesst KEIN leeres .vst3-Gehaeuse stehen, selbst angelegte Ordner verschwinden nur leer, der NAK-41-Riegel greift und zerstoert seine eigene Wiederholungsquelle nicht, eine dritte Artefaktsorte bricht ab. | `py -3.13 tools\eq-copilot\pruefe_installer_gegenpfad.py` | [OK] Exit 0 | 43,60 s | [A18](roh/NAK-96-2271df5-dirty.md#a18) |
| A19 | BandGridZahlen.h ist bytegleich aus den zwei eingefrorenen Gitterfixturen erzeugt; die 64 Live-Gruppen partitionieren die 221 Feinbaender lueckenlos und ueberschneidungsfrei, und die groben Kanten sind bitgleiche Kopien feiner Kanten (kein zweites Filterbank-Gitter). | `py -3.13 tools\eq-copilot\erzeuge_bandgitter_header.py --pruefen` | [OK] Exit 0 | 0,19 s | [A19](roh/NAK-96-2271df5-dirty.md#a19) |
| A20 | Envelope-Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei; jede der 14 Envelope-Regeln hat mindestens ein Negativfixture. | `py -3.13 tools\eq-copilot\erzeuge_envelope_fixtures.py --pruefen` | [OK] Exit 0 | 0,38 s | [A20](roh/NAK-96-2271df5-dirty.md#a20) |
| A21 | v3-Envelope unter Zufall: 20 000 Zufallspuffer bringen den Pruefer nie aus dem Tritt und JEDER angenommene Frame erfuellt jede Kopfregel; 3000 gekippte P2-Payloadbits fallen einzeln an der CRC; feindliche Laengen (0, 15, >Grenze, 0xFFFFFFFF) und die u32-Grenze von 16+payload_len loesen keine Allokation aus; 300 Runden Fragmentierung (byteweise und in Zufallshaeppchen) liefern exakt dieselben Frames; ein kaputter Frame beendet den Strom statt zu resynchronisieren; die Ratengrenze haelt unter Flut. | `cargo test --manifest-path broker/Cargo.toml --test transport_fuzz --color never` | [OK] Exit 0 | 0,38 s | [A21](roh/NAK-96-2271df5-dirty.md#a21) |
| A22 | Ende-zu-Ende ueber die PROBE-Pipe, zwei Sprachen ein Draht: 32 echte C++-Sondenpaare koppeln sich am echten Rust-Listener (32 Control + 32 Telemetry, jede Telemetrieverbindung ueber link_id + challenge + gleiche runtime_nonce), fluten P2 bis die Schleuse mit Cap 2 nachweislich ersetzt, und WAEHRENDDESSEN geht kein einziger P0-Frame verloren; die P0-Antwortlatenz bleibt unter der Schranke. Keine Verbindung wird wegen Envelope, Rate oder P0-Ueberlauf geschlossen. | `py -3.13 tools\eq-copilot\pruefe_ipc_last.py` | [OK] Exit 0 | 10,41 s | [A22](roh/NAK-96-2271df5-dirty.md#a22) |
| A15 | STILLGELEGT - mass bis 28.08.2026 Nakama Suna (NkPr): Passthrough bitgleich ueber drei Samplerates und fuenf Blockgroessen, 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur passive_probe zu; speichern-laden-speichern bytegleich. | `(nicht gefahren)` | [STILLGELEGT] seit 2026-08-28 (S9b/SONDE-007c): das Ziel NakamaSuna ist stillgelegt - Suna ist in Nakama Probeeq aufgegangen (design/abnahmen/2026-08-28-suna-stilllegung-vorgezogen.md). Weder gebaut noch gefahren; die Zeile bleibt sichtbar, damit die Kanonzahl nicht still sinkt. | - | - |
| A16 | Nakama Probeeq (NkAc): heute ebenfalls Passthrough bitgleich (die EQ-DSP kommt in P6), 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur active_probe zu; speichern-laden-speichern bytegleich. Seit der T3-Nacharbeit 29.08.2026 (G1 §4.2) faehrt dasselbe Bein die Gate-7-Kette AM GEBAUTEN BUNDLE Nakama Probeeq - Klasse active_probe, denn die Sondenschale traegt genau EINE Klasse je Uebersetzung: ein sonst gueltiger Stand mit measurement_position=post_fader_contribution kommt ueber setStateInformation read-only zurueck, die Instanz bleibt neutral, die Originalbytes gehen unveraendert an den Host zurueck und ein erneutes Laden waescht nichts; auch eine bereits klassifizierte Instanz faellt beim Nachreichen desselben Standes auf neutral zurueck; Gegenprobe mit insert laedt normal. Nachtrag Runde 2 (Prueferbefund P1): dass Riegel 1 KLASSENUNABHAENGIG sperrt, misst dasselbe Bein zusaetzlich direkt an der oeffentlichen positionErlaubt fuer alle vier Klassen - ohne diese vier Zeilen bliebe die urspruengliche passive_probe-Regression hier unbemerkt, weil kein Ziel mehr NAKAMA_SONDE_PASSIV baut (S9b/SONDE-007c). Die vollstaendige 16er-Matrix samt Bundlevertraegen misst B2. | `eq-copilot\build\plugin\EqCopProbeeqNullTest_artefacts\Release\EqCopProbeeqNullTest.exe` | [OK] Exit 0 | 0,05 s | [A16](roh/NAK-96-2271df5-dirty.md#a16) |
| B1 | Bundle-Identitaet (CIDs, JUCE_VST3_CAN_REPLACE_VST2=0) eingefroren. | `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | [ROT] Exit 1 | 0,08 s | [B1](roh/NAK-96-2271df5-dirty.md#b1) |
| B2 | State-Schema 2: Roundtrip bytegleich, Schema-1-Migration rein und golden, unbekanntes Major read-only mit Originalbytes, Duplicate erkennbar (gleiche instance_id, verschiedene runtime_nonce) und aufloesbar, Host-Dirty; Parametertabelle deckungsgleich mit dem Vertrag; RFC-8785-state_hash bytegleich zu Python und Rust. | `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | [OK] Exit 0 | 0,36 s | [B2](roh/NAK-96-2271df5-dirty.md#b2) |
| B3 | Hostkontext (Anwesenheit, Parameterpunkte, Buslatenz) wird gemessen, nicht geraten; Quellhash-Gate des JUCE-Patches gruen. | `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | [OK] Exit 0 | 0,05 s | [B3](roh/NAK-96-2271df5-dirty.md#b3) |
| B3b | Termin-B-Messgeraet: Passthrough bitgleich, Sprung-/Automations-/Latenzmessung inkl. Fehlalarm-Riegel, Bericht-Rueckweg, 0 Allokationen. | `eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe` | [OK] Exit 0 | 0,08 s | [B3b](roh/NAK-96-2271df5-dirty.md#b3b) |
| B3c | v3-Vertrag: C++ klassifiziert den Fixture-Korpus wie das Manifest (Urteil UND Verletzungsmenge), Bandgitter und Quantisierung bitgleich. | `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | [OK] Exit 0 | 0,22 s | [B3c](roh/NAK-96-2271df5-dirty.md#b3c) |
| B4 | StampedAudioQueue und Ein-Block-Quarantaene: Ganzblockaufnahme bitgleich ueber jeden Ringumlauf, Ueberlauf BEIDER Ringe verwirft den ganzen Block und nie eine Teilmenge, Oversize ueber der Slotkapazitaet faellt fuer die Analyse und laesst Audio unberuehrt, Flush (numSamples 0) ist kein Verlust, Mono dupliziert L, Mehrfach-Tap-Layout traegt; Quarantaene versiegelt erst mit bewiesener Fortsetzung, Seek und Transportkante verwerfen genau EINEN Block, stehende Projektzeit (FL-Teilpuffer) ist kein Bruch, Projektzeit-Ueberlauf und negative Zeit sind behandelt; Worker-Publikation folgt monotonen 50-/250-ms-Deadlines statt Batchzahl, holt nach Pausen nicht auf und laesst wartende Reset-/Frame-Aufrufer vor; verdrahtet: Passthrough bitgleich ueber 18 Blockgroessen von 1 bis 16384, 0 Samples Latenz, kein Tail, 0 Allokationen im Audiothread ueber 4000 Bloecke wechselnder Groesse mit Transportkanten, und die Engine bekommt den Strom bis auf den Block in Quarantaene. | `eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe` | [OK] Exit 0 | 3,36 s | [B4](roh/NAK-96-2271df5-dirty.md#b4) |
| B9 | Fixed-memory Loudness (§48.1): der LoudnessAccumulator deckt sich mit der ausgebauten Vektorrechnung innerhalb ±0,1 LU (Entwurf §49) ueber konstante, rampende, zufaellige und einstuendige Korpora sowie ueber Stille unter dem absoluten Gate; Kurz-LUFS ist BITGLEICH; ein adversarialer Sweep legt 1000 Bloecke in den Grenzbin des relativen Gates und die selbstgemeldete Schranke unsicherheitLu() deckt jeden Lauf; eine Million Zellen laufen mit 0 Allokationen durch, waehrend die Gegenprobe (alte Rechnung) allozert; NaN/Inf-Zellen sind gezaehlt statt still als 0 verbucht und l_j == -70,0 exakt bleibt wertgleich; ueber dem Feingitter traegt ein OBERBAND aus Bins von 1 LU bis ueber lautheit(DBL_MAX) - mit ZWEI Pegeln darin (Korpus des T2-Pruefers), einem adversarialen Sweep im Oberband-Grenzbin, beiden Richtungen der Naht zwischen den Aufloesungen und der Gegenprobe, dass kein Block durch das Raster faellt. | `eq-copilot\build\plugin\EqCopLoudnessGoldenTest_artefacts\Release\EqCopLoudnessGoldenTest.exe` | [OK] Exit 0 | 0,15 s | [B9](roh/NAK-96-2271df5-dirty.md#b9) |
| B5 | FeatureEngine v2 haelt Zeit-, Validity-, Event- und Bandvertraege: Bandgitter und alle 61 Quantisierungsvektoren bitgleich zum v3-Vertrag, Bitmap LSB-first mit Fuellbits 0, FFT gegen Parseval und einen Sinus auf der Binmitte, K-Gewichtung ueber 20 Hz..20 kHz unter 0,1 dB an der BS.1770-Referenzkette; Drop/Seek(laufend UND gestoppt)/Loop-Wrap/moeglicher Straddle/Transportkante/Sampleratewechsel/Neuanlauf/Beweislagewechsel trennen JEDES offene Fenster - auch den K-Filterzustand, bitgleich gemessen - waehrend FL-Teilstuecke mit stehender Projektzeit lokal weiterlaufen, aber kein unbewiesenes Projektintervall oder FFT-Event erzeugen; Frame-Stempel umfassen den echten ueberlappenden FFT-Support und bleiben an int64/u32-Grenzen darstellbar; Drop zaehlt als Segment, alles andere als Epoche; alle sieben Erzeuger-Stempelregeln fallen einzeln und mit eigener Nummer; kein spektraler Fluss ueberbrueckt eine Grenze, der Ereignisring ist fest gedeckelt; LUFS-S trifft die analytisch gerechnete Erwartung unter 0,1 LU; die Nyquist-Kappe greift bei 22,05 kHz wirklich; verdrahtet: alle sieben Gueltigkeitsbits kommen ueber die Hostbruecke durch, der Playhead-Rueckfallweg nachweislich nur zwei. | `eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe` | [OK] Exit 0 | 2,45 s | [B5](roh/NAK-96-2271df5-dirty.md#b5) |
| B6 | Aktiver DSP-Kern liefert die eingefrorene Referenzantwort. | `eq-copilot\build\plugin\EqCopDspGoldenTest_artefacts\Release\EqCopDspGoldenTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B7 | Apply/Revert ist transaktional - kein halber Zustand ueberlebt. | `eq-copilot\build\plugin\EqCopTransactionTest_artefacts\Release\EqCopTransactionTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B10 | v3-Envelope in C++ klassifiziert den Envelope-Korpus wie das Manifest (Urteil UND Verstossmenge, alle 14 Regeln mit Negativfixture); CRC32C trifft die RFC-3720-Vektoren, P0/P1 tragen CRC exakt 0, P2 die Pflichtsumme ueber genau die Payloadbytes; 40 000 Zufallspuffer bringen den Pruefer nie aus dem Tritt und 7671 angenommene EINBIT-Mutanten gueltiger Frames halten jede Kopfregel (reiner Zufall wird praktisch immer abgewiesen - die Invariante braucht deshalb die Mutanten, sonst spraeche sie ueber eine leere Menge), 3000 gekippte P2-Bits fallen einzeln, byteweise Zustellung liefert dieselben 40 Frames und ein kaputter Frame beendet den Strom; Pipetoken trifft das Golden aus §48.3 samt SHA-256- und RFC-4648-Vektoren; P0 verwirft nichts und meldet den 65. Eintrag, P1 koalesziert an der Position und haelt Ereignisse fuer den Reconnect vor, die P2-Schleuse ersetzt den aeltesten ungesendeten Frame, uebergibt 100 000 Frames mit 0 Allokationen (mit Gegenprobe am selben Zaehler) und liefert unter Flut keinen zerrissenen Frame; verdrahtet: Control koppelt Telemetry ueber link_id + challenge, ein ungekoppelter Telemetry-Connect wird geschlossen, der Client verbindet nach Serverneustart von selbst wieder, ein kaputter Envelope vom Server schliesst die Verbindung, und ein P0-Ueberlauf WAEHREND einer stehenden Verbindung schliesst sie ebenfalls statt still zu kuerzen. | `eq-copilot\build\plugin\EqCopIpcTest_artefacts\Release\EqCopIpcTest.exe` | [OK] Exit 0 | 32,32 s | [B10](roh/NAK-96-2271df5-dirty.md#b10) |
| B8 | Lifecycle-Klassifikation §53.5: unclassified beim Laden und audio-neutral; Schema-1 sensor\|pre\|post -> legacy (immer passiv), hub bzw. bestaetigter Schema-2-Main-State -> main; ein Scannerlauf klassifiziert nicht; read-only nimmt die Klassifikation zurueck; Brokerstart nur fuer main mit offenem Editor; die Sondenbundles bleiben bis gueltigem State neutral und werden nie main. | `eq-copilot\build\plugin\EqCopLebenslaufTest_artefacts\Release\EqCopLebenslaufTest.exe` | [OK] Exit 0 | 0,08 s | [B8](roh/NAK-96-2271df5-dirty.md#b8) |


---

## Kanon-Lauf - NAK-96 Kollisionsprobe Lauf 2 (ohne -Bauen)

**Lauf:** 2026-08-29 14:03 | **Runner:** `tools/beweise.ps1` | **Urteil:** ROT - 1 von 32 Kanon-Laeufen fehlgeschlagen | 2 geplante Pruefung(en) noch nicht gebaut | 1 stillgelegte(s) Bein(e), siehe Uebersicht | **Exitcode:** 2 | **Rohausgabe:** [roh/NAK-96-2271df5-dirty-2.md](roh/NAK-96-2271df5-dirty-2.md)

### Kopf - woran gemessen wurde

| Feld | Wert |
|---|---|
| Zeitpunkt | 2026-08-29 14:03:00 +02:00 |
| Rechner | SCHUBBINATOR200 \| Windows 10.0.26200.0 |
| Zweig | master |
| Commit | 2271df5 NEXT-SESSION: Nachtrag Dirigentenrunde 29.08. (FL-Termin, Fragenrunde, SONDE-010 R3, Prozessumbau, Reihenfolge) |
| Commit (voll) | 2271df5bc7ab2ba823ecbf1f7c7dbb68e16984d8 |
| Arbeitsbaum | 7 unbestaetigte Datei(en) - dieser Lauf beweist NICHT allein den Commit |
| JUCE gepinnt | 8.0.9 |
| JUCE auf Platte | 8.0.9-dirty |
| FL Studio | FL Studio 2025 25.2.5.5319 \| FL Studio 2026 26.1.4.5589 |
| cargo | cargo 1.93.1 (083ac5135 2025-12-15) |
| rustc | rustc 1.93.1 (01f6ddf75 2026-02-11) |
| PowerShell | 7.6.5 |

> **VERALTET - dieser Lauf beweist NICHT den aktuellen Quellstand.**
> Mindestens eine Pruefbinaerdatei ist aelter als die Quellen. Neu fahren mit `-Bauen`.

### Uebersicht

| # | Behauptung | Befehl | Ergebnis | Dauer | Rohausgabe |
|---|---|---|---|---|---|
| A1 | Passthrough ist bitgleich; 0 Samples Latenz, 0 Tail; NaN/Inf werden gezaehlt, aber nicht veraendert. | `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | [OK] Exit 0 | 0,06 s | [A1](roh/NAK-96-2271df5-dirty-2.md#a1) |
| A2 | AnalyseEngine deckt sich mit der eingefrorenen Offline-Referenz (Fixture-SHA-256 als Determinismus-Riegel). | `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | [OK] Exit 0 | 11,56 s | [A2](roh/NAK-96-2271df5-dirty-2.md#a2) |
| A3 | Hoer-Markierung bleibt verriegelt: Render/Freilauf bitgleich, Analyse-Abgriff sitzt vor der Faerbung, und der U10-Term ist gedeckt - OHNE Playhead faerbt bei sonst voller Erlaubnis kein Sample (T11, mit Gegenprobe bei laufendem Transport). | `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | [OK] Exit 0 | 6,44 s | [A3](roh/NAK-96-2271df5-dirty-2.md#a3) |
| A4 | Broker-Vertragstests gruen (Framing, Protokoll, Bindung, Aggregat, Server). | `cargo test --manifest-path broker/Cargo.toml --color never` | [OK] Exit 0 | 3,20 s | [A4](roh/NAK-96-2271df5-dirty-2.md#a4) |
| A4b | C++-PipeClient: stop/reconnect und ganze Frames sind zeitlich begrenzt, SQOS ist Identification, Peerbytes sind UTF-8/NUL-sauber, ACK-Sequenzen streng und u64-Zaehler wire-sicher. | `eq-copilot\build\plugin\EqCopPipeClientTest_artefacts\Release\EqCopPipeClientTest.exe` | [OK] Exit 0 | 1,10 s | [A4b](roh/NAK-96-2271df5-dirty-2.md#a4b) |
| A5 | Referenzbein (jsonschema, draft 2020-12): Schema haelt die Engine-Teilmenge ein, Textriegel deckt jede gemessene Kante, jedes Fixture wird wie im Manifest klassifiziert, jede Definition hat ein Negativfixture. | `py -3.13 tools\eq-copilot\pruefe_v3_vertrag.py --abdeckung` | [OK] Exit 0 | 1,76 s | [A5](roh/NAK-96-2271df5-dirty-2.md#a5) |
| A6 | Beide Bandgitter sind bytegleich zur Neuerzeugung; 221 Baender, 64 Gruppen als exakte Partition. | `py -3.13 tools\eq-copilot\erzeuge_bandgitter.py --pruefen` | [OK] Exit 0 | 0,16 s | [A6](roh/NAK-96-2271df5-dirty-2.md#a6) |
| A7 | Quantisierungsvertrag bytegleich zur Neuerzeugung; Rundung, Saettigung und Nichtendliches als Testvektoren. | `py -3.13 tools\eq-copilot\erzeuge_quantisierung.py --pruefen` | [OK] Exit 0 | 0,17 s | [A7](roh/NAK-96-2271df5-dirty-2.md#a7) |
| A8 | Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_v3_fixtures.py --pruefen` | [OK] Exit 0 | 0,21 s | [A8](roh/NAK-96-2271df5-dirty-2.md#a8) |
| A9 | Codegen-Drift ist 0: die Neugenerierung aus dem .fbs ist bytegleich zum committeten C++- und Rust-Code; flatc, C++-Header und Rust-Crate tragen dieselbe gepinnte Version; jedes Tabellenfeld traegt eine explizite Feld-ID. | `py -3.13 tools\eq-copilot\pruefe_flatc_drift.py` | [OK] Exit 0 | 0,33 s | [A9](roh/NAK-96-2271df5-dirty-2.md#a9) |
| A10 | Binaerer Fixture-Korpus und sein MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_fb_fixtures.py --pruefen` | [OK] Exit 0 | 1,15 s | [A10](roh/NAK-96-2271df5-dirty-2.md#a10) |
| A11 | Die fuenf v2-Vertraege (ipc v2, measurement v1, report v1, snapshot v3, aggregat v1) sind gueltiges JSON und gueltige JSON-Schemas; ihre $id-Familie ist eingefroren. | `py -3.13 tools\eq-copilot\pruefe_v2_schemas.py` | [OK] Exit 0 | 0,31 s | [A11](roh/NAK-96-2271df5-dirty-2.md#a11) |
| A12 | Parameterbestand (109 IDs, §53.8) haelt den Vertrag; RFC-8785-Zahlenvektoren tragen den RFC-Text und werden von rfc8785 bestaetigt; State-Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung. | `py -3.13 tools\eq-copilot\erzeuge_state_fixtures.py --pruefen` | [OK] Exit 0 | 0,31 s | [A12](roh/NAK-96-2271df5-dirty-2.md#a12) |
| A13 | Capabilityreport FL: die zehn Bits aus §53.6 entsprechen der v3-Vertragsform und stehen so, wie die Rohdaten der Termine A und B sie tragen; jedes supported hat einen Termin, jedes unsupported seinen festen Fallback. | `py -3.13 tools\eq-copilot\pruefe_host_capabilities.py` | [OK] Exit 0 | 0,26 s | [A13](roh/NAK-96-2271df5-dirty-2.md#a13) |
| A14 | Der gemeinsame Kern traegt keine Bundle-Identitaet: NakamaKern.lib enthaelt keinen eingefrorenen Identitaetswert (Namen, Viercodes, CIDs roh und COM-vertauscht) und genau seine eigenen Uebersetzungseinheiten, kein JUCE-Modulobjekt; die Gegenprobe findet dieselben Werte im gebauten EQ-Copilot-Bundle. | `py -3.13 tools\eq-copilot\pruefe_kern_identitaetsfrei.py` | [OK] Exit 0 | 0,60 s | [A14](roh/NAK-96-2271df5-dirty-2.md#a14) |
| A17 | Installer-Manifest: keine zweite Identitaet (kein Viercode, keine Class-ID), jeder Quellpfad ist der aus Ziel + Identitaetsdatei nachgerechnete BUNDLE-ORDNER, `art` ist eine geschlossene Menge, jedes Ziel genau einmal, Broker aus dem Crate-Namen, Zielverzeichnisse geschuetzt, Signaturzeile behauptet keine Pruefung ohne Mittel, Rueckweg samt NAK-41-Riegel und hash_art vollstaendig; jede Regel faellt an einem verdorbenen Manifest; Ordner-Hash v1 bytegleich in Python und PowerShell, Nicht-ASCII bricht beide Haelften ab. | `py -3.13 tools\eq-copilot\pruefe_installer_manifest.py` | [OK] Exit 0 | 1,04 s | [A17](roh/NAK-96-2271df5-dirty-2.md#a17) |
| A18 | Gegenpfad installieren<->Rueckweg gefahren (Sandbox, nichts installiert): Erstinstallation traegt moduleinfo.json, ein Tausch ersetzt statt zu mischen, -Pruefen sieht den ganzen Ordner, der Rueckweg stellt den Vorzustand bytegleich her und laesst KEIN leeres .vst3-Gehaeuse stehen, selbst angelegte Ordner verschwinden nur leer, der NAK-41-Riegel greift und zerstoert seine eigene Wiederholungsquelle nicht, eine dritte Artefaktsorte bricht ab. | `py -3.13 tools\eq-copilot\pruefe_installer_gegenpfad.py` | [OK] Exit 0 | 42,89 s | [A18](roh/NAK-96-2271df5-dirty-2.md#a18) |
| A19 | BandGridZahlen.h ist bytegleich aus den zwei eingefrorenen Gitterfixturen erzeugt; die 64 Live-Gruppen partitionieren die 221 Feinbaender lueckenlos und ueberschneidungsfrei, und die groben Kanten sind bitgleiche Kopien feiner Kanten (kein zweites Filterbank-Gitter). | `py -3.13 tools\eq-copilot\erzeuge_bandgitter_header.py --pruefen` | [OK] Exit 0 | 0,16 s | [A19](roh/NAK-96-2271df5-dirty-2.md#a19) |
| A20 | Envelope-Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei; jede der 14 Envelope-Regeln hat mindestens ein Negativfixture. | `py -3.13 tools\eq-copilot\erzeuge_envelope_fixtures.py --pruefen` | [OK] Exit 0 | 0,34 s | [A20](roh/NAK-96-2271df5-dirty-2.md#a20) |
| A21 | v3-Envelope unter Zufall: 20 000 Zufallspuffer bringen den Pruefer nie aus dem Tritt und JEDER angenommene Frame erfuellt jede Kopfregel; 3000 gekippte P2-Payloadbits fallen einzeln an der CRC; feindliche Laengen (0, 15, >Grenze, 0xFFFFFFFF) und die u32-Grenze von 16+payload_len loesen keine Allokation aus; 300 Runden Fragmentierung (byteweise und in Zufallshaeppchen) liefern exakt dieselben Frames; ein kaputter Frame beendet den Strom statt zu resynchronisieren; die Ratengrenze haelt unter Flut. | `cargo test --manifest-path broker/Cargo.toml --test transport_fuzz --color never` | [OK] Exit 0 | 0,34 s | [A21](roh/NAK-96-2271df5-dirty-2.md#a21) |
| A22 | Ende-zu-Ende ueber die PROBE-Pipe, zwei Sprachen ein Draht: 32 echte C++-Sondenpaare koppeln sich am echten Rust-Listener (32 Control + 32 Telemetry, jede Telemetrieverbindung ueber link_id + challenge + gleiche runtime_nonce), fluten P2 bis die Schleuse mit Cap 2 nachweislich ersetzt, und WAEHRENDDESSEN geht kein einziger P0-Frame verloren; die P0-Antwortlatenz bleibt unter der Schranke. Keine Verbindung wird wegen Envelope, Rate oder P0-Ueberlauf geschlossen. | `py -3.13 tools\eq-copilot\pruefe_ipc_last.py` | [OK] Exit 0 | 10,42 s | [A22](roh/NAK-96-2271df5-dirty-2.md#a22) |
| A15 | STILLGELEGT - mass bis 28.08.2026 Nakama Suna (NkPr): Passthrough bitgleich ueber drei Samplerates und fuenf Blockgroessen, 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur passive_probe zu; speichern-laden-speichern bytegleich. | `(nicht gefahren)` | [STILLGELEGT] seit 2026-08-28 (S9b/SONDE-007c): das Ziel NakamaSuna ist stillgelegt - Suna ist in Nakama Probeeq aufgegangen (design/abnahmen/2026-08-28-suna-stilllegung-vorgezogen.md). Weder gebaut noch gefahren; die Zeile bleibt sichtbar, damit die Kanonzahl nicht still sinkt. | - | - |
| A16 | Nakama Probeeq (NkAc): heute ebenfalls Passthrough bitgleich (die EQ-DSP kommt in P6), 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur active_probe zu; speichern-laden-speichern bytegleich. Seit der T3-Nacharbeit 29.08.2026 (G1 §4.2) faehrt dasselbe Bein die Gate-7-Kette AM GEBAUTEN BUNDLE Nakama Probeeq - Klasse active_probe, denn die Sondenschale traegt genau EINE Klasse je Uebersetzung: ein sonst gueltiger Stand mit measurement_position=post_fader_contribution kommt ueber setStateInformation read-only zurueck, die Instanz bleibt neutral, die Originalbytes gehen unveraendert an den Host zurueck und ein erneutes Laden waescht nichts; auch eine bereits klassifizierte Instanz faellt beim Nachreichen desselben Standes auf neutral zurueck; Gegenprobe mit insert laedt normal. Nachtrag Runde 2 (Prueferbefund P1): dass Riegel 1 KLASSENUNABHAENGIG sperrt, misst dasselbe Bein zusaetzlich direkt an der oeffentlichen positionErlaubt fuer alle vier Klassen - ohne diese vier Zeilen bliebe die urspruengliche passive_probe-Regression hier unbemerkt, weil kein Ziel mehr NAKAMA_SONDE_PASSIV baut (S9b/SONDE-007c). Die vollstaendige 16er-Matrix samt Bundlevertraegen misst B2. | `eq-copilot\build\plugin\EqCopProbeeqNullTest_artefacts\Release\EqCopProbeeqNullTest.exe` | [OK] Exit 0 | 0,05 s | [A16](roh/NAK-96-2271df5-dirty-2.md#a16) |
| B1 | Bundle-Identitaet (CIDs, JUCE_VST3_CAN_REPLACE_VST2=0) eingefroren. | `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | [ROT] Exit 1 | 0,07 s | [B1](roh/NAK-96-2271df5-dirty-2.md#b1) |
| B2 | State-Schema 2: Roundtrip bytegleich, Schema-1-Migration rein und golden, unbekanntes Major read-only mit Originalbytes, Duplicate erkennbar (gleiche instance_id, verschiedene runtime_nonce) und aufloesbar, Host-Dirty; Parametertabelle deckungsgleich mit dem Vertrag; RFC-8785-state_hash bytegleich zu Python und Rust. | `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | [OK] Exit 0 | 0,34 s | [B2](roh/NAK-96-2271df5-dirty-2.md#b2) |
| B3 | Hostkontext (Anwesenheit, Parameterpunkte, Buslatenz) wird gemessen, nicht geraten; Quellhash-Gate des JUCE-Patches gruen. | `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | [OK] Exit 0 | 0,05 s | [B3](roh/NAK-96-2271df5-dirty-2.md#b3) |
| B3b | Termin-B-Messgeraet: Passthrough bitgleich, Sprung-/Automations-/Latenzmessung inkl. Fehlalarm-Riegel, Bericht-Rueckweg, 0 Allokationen. | `eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe` | [OK] Exit 0 | 0,10 s | [B3b](roh/NAK-96-2271df5-dirty-2.md#b3b) |
| B3c | v3-Vertrag: C++ klassifiziert den Fixture-Korpus wie das Manifest (Urteil UND Verletzungsmenge), Bandgitter und Quantisierung bitgleich. | `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | [OK] Exit 0 | 0,21 s | [B3c](roh/NAK-96-2271df5-dirty-2.md#b3c) |
| B4 | StampedAudioQueue und Ein-Block-Quarantaene: Ganzblockaufnahme bitgleich ueber jeden Ringumlauf, Ueberlauf BEIDER Ringe verwirft den ganzen Block und nie eine Teilmenge, Oversize ueber der Slotkapazitaet faellt fuer die Analyse und laesst Audio unberuehrt, Flush (numSamples 0) ist kein Verlust, Mono dupliziert L, Mehrfach-Tap-Layout traegt; Quarantaene versiegelt erst mit bewiesener Fortsetzung, Seek und Transportkante verwerfen genau EINEN Block, stehende Projektzeit (FL-Teilpuffer) ist kein Bruch, Projektzeit-Ueberlauf und negative Zeit sind behandelt; Worker-Publikation folgt monotonen 50-/250-ms-Deadlines statt Batchzahl, holt nach Pausen nicht auf und laesst wartende Reset-/Frame-Aufrufer vor; verdrahtet: Passthrough bitgleich ueber 18 Blockgroessen von 1 bis 16384, 0 Samples Latenz, kein Tail, 0 Allokationen im Audiothread ueber 4000 Bloecke wechselnder Groesse mit Transportkanten, und die Engine bekommt den Strom bis auf den Block in Quarantaene. | `eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe` | [OK] Exit 0 | 3,38 s | [B4](roh/NAK-96-2271df5-dirty-2.md#b4) |
| B9 | Fixed-memory Loudness (§48.1): der LoudnessAccumulator deckt sich mit der ausgebauten Vektorrechnung innerhalb ±0,1 LU (Entwurf §49) ueber konstante, rampende, zufaellige und einstuendige Korpora sowie ueber Stille unter dem absoluten Gate; Kurz-LUFS ist BITGLEICH; ein adversarialer Sweep legt 1000 Bloecke in den Grenzbin des relativen Gates und die selbstgemeldete Schranke unsicherheitLu() deckt jeden Lauf; eine Million Zellen laufen mit 0 Allokationen durch, waehrend die Gegenprobe (alte Rechnung) allozert; NaN/Inf-Zellen sind gezaehlt statt still als 0 verbucht und l_j == -70,0 exakt bleibt wertgleich; ueber dem Feingitter traegt ein OBERBAND aus Bins von 1 LU bis ueber lautheit(DBL_MAX) - mit ZWEI Pegeln darin (Korpus des T2-Pruefers), einem adversarialen Sweep im Oberband-Grenzbin, beiden Richtungen der Naht zwischen den Aufloesungen und der Gegenprobe, dass kein Block durch das Raster faellt. | `eq-copilot\build\plugin\EqCopLoudnessGoldenTest_artefacts\Release\EqCopLoudnessGoldenTest.exe` | [OK] Exit 0 | 0,16 s | [B9](roh/NAK-96-2271df5-dirty-2.md#b9) |
| B5 | FeatureEngine v2 haelt Zeit-, Validity-, Event- und Bandvertraege: Bandgitter und alle 61 Quantisierungsvektoren bitgleich zum v3-Vertrag, Bitmap LSB-first mit Fuellbits 0, FFT gegen Parseval und einen Sinus auf der Binmitte, K-Gewichtung ueber 20 Hz..20 kHz unter 0,1 dB an der BS.1770-Referenzkette; Drop/Seek(laufend UND gestoppt)/Loop-Wrap/moeglicher Straddle/Transportkante/Sampleratewechsel/Neuanlauf/Beweislagewechsel trennen JEDES offene Fenster - auch den K-Filterzustand, bitgleich gemessen - waehrend FL-Teilstuecke mit stehender Projektzeit lokal weiterlaufen, aber kein unbewiesenes Projektintervall oder FFT-Event erzeugen; Frame-Stempel umfassen den echten ueberlappenden FFT-Support und bleiben an int64/u32-Grenzen darstellbar; Drop zaehlt als Segment, alles andere als Epoche; alle sieben Erzeuger-Stempelregeln fallen einzeln und mit eigener Nummer; kein spektraler Fluss ueberbrueckt eine Grenze, der Ereignisring ist fest gedeckelt; LUFS-S trifft die analytisch gerechnete Erwartung unter 0,1 LU; die Nyquist-Kappe greift bei 22,05 kHz wirklich; verdrahtet: alle sieben Gueltigkeitsbits kommen ueber die Hostbruecke durch, der Playhead-Rueckfallweg nachweislich nur zwei. | `eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe` | [OK] Exit 0 | 2,46 s | [B5](roh/NAK-96-2271df5-dirty-2.md#b5) |
| B6 | Aktiver DSP-Kern liefert die eingefrorene Referenzantwort. | `eq-copilot\build\plugin\EqCopDspGoldenTest_artefacts\Release\EqCopDspGoldenTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B7 | Apply/Revert ist transaktional - kein halber Zustand ueberlebt. | `eq-copilot\build\plugin\EqCopTransactionTest_artefacts\Release\EqCopTransactionTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B10 | v3-Envelope in C++ klassifiziert den Envelope-Korpus wie das Manifest (Urteil UND Verstossmenge, alle 14 Regeln mit Negativfixture); CRC32C trifft die RFC-3720-Vektoren, P0/P1 tragen CRC exakt 0, P2 die Pflichtsumme ueber genau die Payloadbytes; 40 000 Zufallspuffer bringen den Pruefer nie aus dem Tritt und 7671 angenommene EINBIT-Mutanten gueltiger Frames halten jede Kopfregel (reiner Zufall wird praktisch immer abgewiesen - die Invariante braucht deshalb die Mutanten, sonst spraeche sie ueber eine leere Menge), 3000 gekippte P2-Bits fallen einzeln, byteweise Zustellung liefert dieselben 40 Frames und ein kaputter Frame beendet den Strom; Pipetoken trifft das Golden aus §48.3 samt SHA-256- und RFC-4648-Vektoren; P0 verwirft nichts und meldet den 65. Eintrag, P1 koalesziert an der Position und haelt Ereignisse fuer den Reconnect vor, die P2-Schleuse ersetzt den aeltesten ungesendeten Frame, uebergibt 100 000 Frames mit 0 Allokationen (mit Gegenprobe am selben Zaehler) und liefert unter Flut keinen zerrissenen Frame; verdrahtet: Control koppelt Telemetry ueber link_id + challenge, ein ungekoppelter Telemetry-Connect wird geschlossen, der Client verbindet nach Serverneustart von selbst wieder, ein kaputter Envelope vom Server schliesst die Verbindung, und ein P0-Ueberlauf WAEHREND einer stehenden Verbindung schliesst sie ebenfalls statt still zu kuerzen. | `eq-copilot\build\plugin\EqCopIpcTest_artefacts\Release\EqCopIpcTest.exe` | [OK] Exit 0 | 32,79 s | [B10](roh/NAK-96-2271df5-dirty-2.md#b10) |
| B8 | Lifecycle-Klassifikation §53.5: unclassified beim Laden und audio-neutral; Schema-1 sensor\|pre\|post -> legacy (immer passiv), hub bzw. bestaetigter Schema-2-Main-State -> main; ein Scannerlauf klassifiziert nicht; read-only nimmt die Klassifikation zurueck; Brokerstart nur fuer main mit offenem Editor; die Sondenbundles bleiben bis gueltigem State neutral und werden nie main. | `eq-copilot\build\plugin\EqCopLebenslaufTest_artefacts\Release\EqCopLebenslaufTest.exe` | [OK] Exit 0 | 0,08 s | [B8](roh/NAK-96-2271df5-dirty-2.md#b8) |

