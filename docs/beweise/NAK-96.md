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

**Befehl:** `py -3.13 <skript> docs/beweise/NAK-96.md` — das Skript übernimmt die
Regex 1:1 aus `tools/plan/planstand.py` Zeile 66 und bildet `kanon_lesen()` nach.

```text
Datei:   docs\beweise\NAK-96.md
Treffer: 1
  GRUEN  32/32 Kanon
kanon_lesen() wuerde liefern: Kanon 32/32 gruen

Alle Urteilstexte in der Datei:
  ROT - 1 von 32 Kanon-Laeufen fehlgeschlagen
  ROT - 1 von 32 Kanon-Laeufen fehlgeschlagen
  ROT - 1 von 32 Kanon-Laeufen fehlgeschlagen
  GRUEN - 32/32 Kanon-Laeufe bestanden
```

**Gemessen:** die Kanon-Zahl `32/32 grün` des Abschlusslaufs steht weiterhin im
**Manifest** und wird von der unveränderten Regex gefunden. Der Umbau kostet den
Planstand nichts.

`planstand.py` direkt danach, ohne neue Warnung (das Skript sammelt Warnungen
und gibt sie vor der Erfolgszeile aus — hier keine):

```text
geschrieben: docs\PLAN-STAND.md (14 abgenommen, 5 gebaut, 38 gesamt, aus d993894)
exit=0
```

**Nebenbefund, außerhalb der Ticketgrenze:** die drei `ROT`-Zeilen der
Probeläufe trifft die Regex **nicht** — sie verlangt `(\d+)/(\d+)`, der Runner
schreibt im ROT-Fall aber `1 von 32 Kanon-Laeufen`. Die Regex nennt `ROT` als
Alternative und kann sie nie treffen; der Planstand zeigt bei einem roten Kanon
gar keine Zahl statt einer roten. Das ist Bestandsverhalten, kein Neuschaden
dieses Tickets (die Urteilstexte sind laut Auftrag ausdrücklich unverändert
geblieben) — abgelegt als **NAK-97** in `docs/offene-punkte.md`.

**Falle für Manifestschreiber, hier bewusst in Kauf genommen:** §5.5 zitiert die
Lauf-Zeile des Abschlusslaufs im Klartext, und die Regex trifft auch dieses
Zitat — nach dem Nachtrag findet sie zwei Treffer statt einem. `kanon_lesen()`
nimmt den **letzten** Treffer, und das ist hier der echte angehängte
Kanon-Abschnitt am Dateiende; beide tragen ohnehin `32/32 grün`. Wer eine
Lauf-Zeile **nach** dem letzten Kanon-Abschnitt zitiert, verschiebt dagegen die
Zahl, die der Planstand liest. Zitate von Lauf-Zeilen gehören deshalb vor die
angehängten Abschnitte, nie dahinter.

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

Gemessen nach dem Abschlusslauf (§5.5), mit `wc -l` bzw. `wc -c`:

```text
docs/beweise/NAK-96.md                           602 Zeilen      90880 Bytes
docs/beweise/roh/NAK-96-d993894.md              2944 Zeilen     199579 Bytes
docs/beweise/SONDE-010.md                      19505 Zeilen    1307556 Bytes
docs/beweise/SONDE-007b.md                     24796 Zeilen    1581024 Bytes
```

Die 602 Zeilen sind der Stand unmittelbar nach dem Abschlusslauf, also vor dem
Nachtragen von §5.1, §5.5 und §7. Der endgültige Stand dieser Datei:

```text
$ wc -l < docs/beweise/NAK-96.md
793
$ grep -n '^## Kanon-Lauf' docs/beweise/NAK-96.md
533:## Kanon-Lauf - NAK-96 Kollisionsprobe Lauf 1 (ohne -Bauen)
600:## Kanon-Lauf - NAK-96 Kollisionsprobe Lauf 2 (ohne -Bauen)
667:## Kanon-Lauf - NAK-96
732:## Kanon-Lauf - NAK-96 Abschlusslauf 2
```

**Das ist die Zahl, um die es dem Auftrag geht:** ein Kanon-Abschnitt im
Manifest ist **62 bis 67 Zeilen** lang (die vier Abschnitte oben: 67, 67, 65,
62 — die längeren tragen zusätzlich den VERALTET-Hinweis der Läufe ohne
`-Bauen`). Vorher war derselbe Abschnitt die vollen **2944 Zeilen**, die jetzt
in der Roh-Datei liegen — Faktor rund **45**. Die übrigen 532 Zeilen dieser
Datei sind der Lesetext des Tickets selbst (§1–§7 samt den Rohausgaben der
Proben), nicht Kanon-Ausgabe.

Zum Vergleich die beiden Manifeste, die der Auftrag als unlesbar nennt:
`SONDE-010.md` mit 19 505 und `SONDE-007b.md` mit 24 796 Zeilen. Sie bleiben
unangetastet — der Umbau wirkt ab dem nächsten Lauf, nicht rückwirkend.

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

Zwei Läufe, beide auf einem **sauberen** Arbeitsbaum und einem committeten
Stand. Beide sind hier aufgeführt, weil der erste rot war; ein zweiter Lauf
nach einem roten ist nur dann ehrlich, wenn beide im Manifest stehen.

**Befehl (beide Male):**

```powershell
pwsh -NoProfile -File tools/beweise.ps1 -Bauen -Ziel docs/beweise/NAK-96.md -Anhaengen -Titel '…'
```

#### Abschlusslauf 1 — Stand `8a1ea8a`, ROT

```text
ROT - 1 von 32 Kanon-Laeufen fehlgeschlagen | 2 geplante Pruefung(en) noch nicht gebaut | 1 stillgelegte(s) Bein(e), siehe Uebersicht
Manifest:   docs\beweise\NAK-96.md
Rohausgabe: docs\beweise\roh\NAK-96-8a1ea8a.md
exit=2
Dauer: 3,2 min
```

Rot war **B10 `EqCopIpcTest`**, Exit 1, an genau einer Zeile:

```text
  ok      der Erzeuger traf den beanspruchten Slot WIRKLICH — und hat ihn nicht beschrieben  [11234 uebersprungene Positionen]
  FEHLER  der NEUESTE Frame faellt dabei NIE — es weicht der aelteste wartende (replace-oldest, §53.9)  [2 neueste wegen fremden Anspruchs verworfen]
```

Das ist der P2-Slot-Anspruch aus dem NAK-92/NAK-95-Bereich, **nicht** NAK-96:
dieses Ticket ändert keine einzige Zeile C++, Rust oder Python — nur die
Dateiausgabe des Runners und die git-Argumente. Zwei Belege dafür, dass es
lastabhängig ist und nicht am Stand hängt:

1. Dieselbe Binärdatei, unmittelbar danach fünfmal einzeln gefahren, ohne
   parallelen Bau:

   ```text
   Lauf 1: Exit 0 |   ok      der NEUESTE Frame faellt dabei NIE — es weicht der aelteste wartende (replace-oldest, §53.9)  [0 neueste wegen fremden Anspruchs verworfen]
   Lauf 2: Exit 0 |   ok      … [0 neueste wegen fremden Anspruchs verworfen]
   Lauf 3: Exit 0 |   ok      … [0 neueste wegen fremden Anspruchs verworfen]
   Lauf 4: Exit 0 |   ok      … [0 neueste wegen fremden Anspruchs verworfen]
   Lauf 5: Exit 0 |   ok      … [0 neueste wegen fremden Anspruchs verworfen]
   ```

2. In den beiden Probeläufen aus §5.2 (ältere Binaries, gleiche Quelle) war
   B10 grün — dort fiel B1 aus einem anderen, ebenfalls NAK-96-fremden Grund.

Der Befund ist als **NAK-98** in `docs/offene-punkte.md` abgelegt und
absichtlich **nicht** repariert: er liegt außerhalb der Ticketgrenze.

#### Abschlusslauf 2 — Stand `d993894`, GRÜN und beglaubigt

```text
GRUEN - 32/32 Kanon-Laeufe bestanden | 2 geplante Pruefung(en) noch nicht gebaut | 1 stillgelegte(s) Bein(e), siehe Uebersicht
Manifest:   docs\beweise\NAK-96.md
Rohausgabe: docs\beweise\roh\NAK-96-d993894.md
exit=0
Dauer: 2,8 min
```

Kopf des Abschnitts, wie er im Manifest steht — Arbeitsbaum `sauber`, kein
`-dirty` im Dateinamen, kein VERALTET-Hinweis, also **beglaubigt**:

```text
**Lauf:** 2026-08-29 14:16 | **Runner:** `tools/beweise.ps1` | **Urteil:** GRUEN - 32/32 Kanon-Laeufe bestanden | 2 geplante Pruefung(en) noch nicht gebaut | 1 stillgelegte(s) Bein(e), siehe Uebersicht | **Exitcode:** 0 | **Rohausgabe:** [roh/NAK-96-d993894.md](roh/NAK-96-d993894.md)
```

und eine Zeile der Übersicht, die jetzt in die Roh-Datei zeigt:

```text
| A1 | Passthrough ist bitgleich; 0 Samples Latenz, 0 Tail; NaN/Inf werden gezaehlt, aber nicht veraendert. | `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | [OK] Exit 0 | 0,07 s | [A1](roh/NAK-96-d993894.md#a1) |
```

**Beglaubigungslogik unverändert (Prüfliste D):** derselbe Urteilstext,
dieselbe Exitcode-Reihenfolge (2 vor 3 vor 4), derselbe `VERALTET`-Riegel — in
§5.2 hat er bei den Läufen ohne `-Bauen` korrekt angeschlagen, hier bei
gebautem Stand korrekt geschwiegen.

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

| # | SHA | Inhalt |
|---|---|---|
| 1 | `ad7ac99` | Runner: Aufteilung Manifest/Rohausgabe, Roh-Dateiname mit `-dirty`- und Zählsuffix, `--no-optional-locks` in `Git-Wert` und im JUCE-`describe`, Kommentarkopf und `.PARAMETER` |
| 2 | `1a85ee1` | Cockpit: `--no-optional-locks` in `Invoke-GitText` |
| 3 | `f02a91d` | `CLAUDE.md`, `docs/beweise/VORLAGE.md` §3, `docs/offene-punkte.md` (NAK-97), dieses Manifest, Roh-Dateien der Kollisions- und Bruchprobe |
| 4 | `8a1ea8a` | Planstand auf `f02a91d` neu gerechnet (Arbeitsbaum sauber für den Abschlusslauf) |
| 5 | `d993894` | Abschlusslauf 1 (ROT, B10 lastabhängig) samt Roh-Datei — als Beleg festgehalten, nicht verworfen |
| 6 | dieser Commit | Abschlusslauf 2 (GRÜN, beglaubigt) samt Roh-Datei, §5.1/§5.3/§5.5/§7 dieses Manifests, NAK-98, Planstand |

Jeder Commit mit explizitem Pathspec; kein `git add -A`, kein `--amend`, kein
`reset`. Der Arbeitsbaum war vor jedem Commit nur mit eigenen Änderungen
belegt (`git status --short` jeweils gelesen); fremde uncommittete Dateien gab
es in dieser Sitzung nicht.

### Was nicht erledigt ist

- **Bestehende Manifeste** (`SONDE-007b.md` 24 796 Zeilen, `SONDE-010.md`
  19 505 Zeilen) bleiben unverändert — so verlangt es der Auftrag. Der Umbau
  wirkt ab dem nächsten Lauf.
- **NAK-93** (Broker wird vom Kanon nicht gebaut) und **NAK-94**
  (A17-Hashvergleich nach Relink) sind nicht angefasst; beide sind
  ausdrücklich außerhalb der Ticketgrenze. A17 war in beiden Abschlussläufen
  grün, der NAK-94-Fall trat hier also nicht ein.
- **NAK-97** (`planstand.py` ohne `--no-optional-locks`; `KANON`-Regex trifft
  den ROT-Wortlaut nie) und **NAK-98** (B10 unter Last) sind gemessen und
  datiert abgelegt, nicht repariert — beide liegen außerhalb der Grenze.

### Cockpit nach den Commits

`pwsh -NoProfile -File tools/dirigent/cockpit.ps1 -Plan` — vollständige
Planansicht, Exit 0 (vor den Commits meldete dasselbe Cockpit korrekt
`PLANQUELLEN NICHT FRISCH`, §5.4):

```text
NAKAMA PLAN · 14 / 38 fertig · 24 offen

BEWEISEN STATT BEHAUPTEN
✓ Einen vollständigen, wiederholbaren Beweislauf schaffen
…
Fertig bedeutet: umgesetzt, aktuell belegt und auf der geforderten Stufe abgenommen.
exit=0
```

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


---

## Kanon-Lauf - NAK-96

**Lauf:** 2026-08-29 14:10 | **Runner:** `tools/beweise.ps1` | **Urteil:** ROT - 1 von 32 Kanon-Laeufen fehlgeschlagen | 2 geplante Pruefung(en) noch nicht gebaut | 1 stillgelegte(s) Bein(e), siehe Uebersicht | **Exitcode:** 2 | **Rohausgabe:** [roh/NAK-96-8a1ea8a.md](roh/NAK-96-8a1ea8a.md)

### Kopf - woran gemessen wurde

| Feld | Wert |
|---|---|
| Zeitpunkt | 2026-08-29 14:10:05 +02:00 |
| Rechner | SCHUBBINATOR200 \| Windows 10.0.26200.0 |
| Zweig | master |
| Commit | 8a1ea8a NAK-96: Planstand auf f02a91d neu gerechnet |
| Commit (voll) | 8a1ea8af33adca15d1ac8b5d519dc6fd2c3d8bd4 |
| Arbeitsbaum | sauber |
| JUCE gepinnt | 8.0.9 |
| JUCE auf Platte | 8.0.9-dirty |
| FL Studio | FL Studio 2025 25.2.5.5319 \| FL Studio 2026 26.1.4.5589 |
| cargo | cargo 1.93.1 (083ac5135 2025-12-15) |
| rustc | rustc 1.93.1 (01f6ddf75 2026-02-11) |
| PowerShell | 7.6.5 |
| cmake | cmake version 3.31.6-msvc6 |

### Uebersicht

| # | Behauptung | Befehl | Ergebnis | Dauer | Rohausgabe |
|---|---|---|---|---|---|
| A1 | Passthrough ist bitgleich; 0 Samples Latenz, 0 Tail; NaN/Inf werden gezaehlt, aber nicht veraendert. | `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | [OK] Exit 0 | 0,06 s | [A1](roh/NAK-96-8a1ea8a.md#a1) |
| A2 | AnalyseEngine deckt sich mit der eingefrorenen Offline-Referenz (Fixture-SHA-256 als Determinismus-Riegel). | `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | [OK] Exit 0 | 12,25 s | [A2](roh/NAK-96-8a1ea8a.md#a2) |
| A3 | Hoer-Markierung bleibt verriegelt: Render/Freilauf bitgleich, Analyse-Abgriff sitzt vor der Faerbung, und der U10-Term ist gedeckt - OHNE Playhead faerbt bei sonst voller Erlaubnis kein Sample (T11, mit Gegenprobe bei laufendem Transport). | `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | [OK] Exit 0 | 6,38 s | [A3](roh/NAK-96-8a1ea8a.md#a3) |
| A4 | Broker-Vertragstests gruen (Framing, Protokoll, Bindung, Aggregat, Server). | `cargo test --manifest-path broker/Cargo.toml --color never` | [OK] Exit 0 | 3,21 s | [A4](roh/NAK-96-8a1ea8a.md#a4) |
| A4b | C++-PipeClient: stop/reconnect und ganze Frames sind zeitlich begrenzt, SQOS ist Identification, Peerbytes sind UTF-8/NUL-sauber, ACK-Sequenzen streng und u64-Zaehler wire-sicher. | `eq-copilot\build\plugin\EqCopPipeClientTest_artefacts\Release\EqCopPipeClientTest.exe` | [OK] Exit 0 | 1,11 s | [A4b](roh/NAK-96-8a1ea8a.md#a4b) |
| A5 | Referenzbein (jsonschema, draft 2020-12): Schema haelt die Engine-Teilmenge ein, Textriegel deckt jede gemessene Kante, jedes Fixture wird wie im Manifest klassifiziert, jede Definition hat ein Negativfixture. | `py -3.13 tools\eq-copilot\pruefe_v3_vertrag.py --abdeckung` | [OK] Exit 0 | 1,79 s | [A5](roh/NAK-96-8a1ea8a.md#a5) |
| A6 | Beide Bandgitter sind bytegleich zur Neuerzeugung; 221 Baender, 64 Gruppen als exakte Partition. | `py -3.13 tools\eq-copilot\erzeuge_bandgitter.py --pruefen` | [OK] Exit 0 | 0,16 s | [A6](roh/NAK-96-8a1ea8a.md#a6) |
| A7 | Quantisierungsvertrag bytegleich zur Neuerzeugung; Rundung, Saettigung und Nichtendliches als Testvektoren. | `py -3.13 tools\eq-copilot\erzeuge_quantisierung.py --pruefen` | [OK] Exit 0 | 0,16 s | [A7](roh/NAK-96-8a1ea8a.md#a7) |
| A8 | Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_v3_fixtures.py --pruefen` | [OK] Exit 0 | 0,22 s | [A8](roh/NAK-96-8a1ea8a.md#a8) |
| A9 | Codegen-Drift ist 0: die Neugenerierung aus dem .fbs ist bytegleich zum committeten C++- und Rust-Code; flatc, C++-Header und Rust-Crate tragen dieselbe gepinnte Version; jedes Tabellenfeld traegt eine explizite Feld-ID. | `py -3.13 tools\eq-copilot\pruefe_flatc_drift.py` | [OK] Exit 0 | 0,30 s | [A9](roh/NAK-96-8a1ea8a.md#a9) |
| A10 | Binaerer Fixture-Korpus und sein MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_fb_fixtures.py --pruefen` | [OK] Exit 0 | 1,28 s | [A10](roh/NAK-96-8a1ea8a.md#a10) |
| A11 | Die fuenf v2-Vertraege (ipc v2, measurement v1, report v1, snapshot v3, aggregat v1) sind gueltiges JSON und gueltige JSON-Schemas; ihre $id-Familie ist eingefroren. | `py -3.13 tools\eq-copilot\pruefe_v2_schemas.py` | [OK] Exit 0 | 0,29 s | [A11](roh/NAK-96-8a1ea8a.md#a11) |
| A12 | Parameterbestand (109 IDs, §53.8) haelt den Vertrag; RFC-8785-Zahlenvektoren tragen den RFC-Text und werden von rfc8785 bestaetigt; State-Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung. | `py -3.13 tools\eq-copilot\erzeuge_state_fixtures.py --pruefen` | [OK] Exit 0 | 0,31 s | [A12](roh/NAK-96-8a1ea8a.md#a12) |
| A13 | Capabilityreport FL: die zehn Bits aus §53.6 entsprechen der v3-Vertragsform und stehen so, wie die Rohdaten der Termine A und B sie tragen; jedes supported hat einen Termin, jedes unsupported seinen festen Fallback. | `py -3.13 tools\eq-copilot\pruefe_host_capabilities.py` | [OK] Exit 0 | 0,26 s | [A13](roh/NAK-96-8a1ea8a.md#a13) |
| A14 | Der gemeinsame Kern traegt keine Bundle-Identitaet: NakamaKern.lib enthaelt keinen eingefrorenen Identitaetswert (Namen, Viercodes, CIDs roh und COM-vertauscht) und genau seine eigenen Uebersetzungseinheiten, kein JUCE-Modulobjekt; die Gegenprobe findet dieselben Werte im gebauten EQ-Copilot-Bundle. | `py -3.13 tools\eq-copilot\pruefe_kern_identitaetsfrei.py` | [OK] Exit 0 | 0,60 s | [A14](roh/NAK-96-8a1ea8a.md#a14) |
| A17 | Installer-Manifest: keine zweite Identitaet (kein Viercode, keine Class-ID), jeder Quellpfad ist der aus Ziel + Identitaetsdatei nachgerechnete BUNDLE-ORDNER, `art` ist eine geschlossene Menge, jedes Ziel genau einmal, Broker aus dem Crate-Namen, Zielverzeichnisse geschuetzt, Signaturzeile behauptet keine Pruefung ohne Mittel, Rueckweg samt NAK-41-Riegel und hash_art vollstaendig; jede Regel faellt an einem verdorbenen Manifest; Ordner-Hash v1 bytegleich in Python und PowerShell, Nicht-ASCII bricht beide Haelften ab. | `py -3.13 tools\eq-copilot\pruefe_installer_manifest.py` | [OK] Exit 0 | 1,02 s | [A17](roh/NAK-96-8a1ea8a.md#a17) |
| A18 | Gegenpfad installieren<->Rueckweg gefahren (Sandbox, nichts installiert): Erstinstallation traegt moduleinfo.json, ein Tausch ersetzt statt zu mischen, -Pruefen sieht den ganzen Ordner, der Rueckweg stellt den Vorzustand bytegleich her und laesst KEIN leeres .vst3-Gehaeuse stehen, selbst angelegte Ordner verschwinden nur leer, der NAK-41-Riegel greift und zerstoert seine eigene Wiederholungsquelle nicht, eine dritte Artefaktsorte bricht ab. | `py -3.13 tools\eq-copilot\pruefe_installer_gegenpfad.py` | [OK] Exit 0 | 44,54 s | [A18](roh/NAK-96-8a1ea8a.md#a18) |
| A19 | BandGridZahlen.h ist bytegleich aus den zwei eingefrorenen Gitterfixturen erzeugt; die 64 Live-Gruppen partitionieren die 221 Feinbaender lueckenlos und ueberschneidungsfrei, und die groben Kanten sind bitgleiche Kopien feiner Kanten (kein zweites Filterbank-Gitter). | `py -3.13 tools\eq-copilot\erzeuge_bandgitter_header.py --pruefen` | [OK] Exit 0 | 0,17 s | [A19](roh/NAK-96-8a1ea8a.md#a19) |
| A20 | Envelope-Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei; jede der 14 Envelope-Regeln hat mindestens ein Negativfixture. | `py -3.13 tools\eq-copilot\erzeuge_envelope_fixtures.py --pruefen` | [OK] Exit 0 | 0,37 s | [A20](roh/NAK-96-8a1ea8a.md#a20) |
| A21 | v3-Envelope unter Zufall: 20 000 Zufallspuffer bringen den Pruefer nie aus dem Tritt und JEDER angenommene Frame erfuellt jede Kopfregel; 3000 gekippte P2-Payloadbits fallen einzeln an der CRC; feindliche Laengen (0, 15, >Grenze, 0xFFFFFFFF) und die u32-Grenze von 16+payload_len loesen keine Allokation aus; 300 Runden Fragmentierung (byteweise und in Zufallshaeppchen) liefern exakt dieselben Frames; ein kaputter Frame beendet den Strom statt zu resynchronisieren; die Ratengrenze haelt unter Flut. | `cargo test --manifest-path broker/Cargo.toml --test transport_fuzz --color never` | [OK] Exit 0 | 0,34 s | [A21](roh/NAK-96-8a1ea8a.md#a21) |
| A22 | Ende-zu-Ende ueber die PROBE-Pipe, zwei Sprachen ein Draht: 32 echte C++-Sondenpaare koppeln sich am echten Rust-Listener (32 Control + 32 Telemetry, jede Telemetrieverbindung ueber link_id + challenge + gleiche runtime_nonce), fluten P2 bis die Schleuse mit Cap 2 nachweislich ersetzt, und WAEHRENDDESSEN geht kein einziger P0-Frame verloren; die P0-Antwortlatenz bleibt unter der Schranke. Keine Verbindung wird wegen Envelope, Rate oder P0-Ueberlauf geschlossen. | `py -3.13 tools\eq-copilot\pruefe_ipc_last.py` | [OK] Exit 0 | 10,38 s | [A22](roh/NAK-96-8a1ea8a.md#a22) |
| A15 | STILLGELEGT - mass bis 28.08.2026 Nakama Suna (NkPr): Passthrough bitgleich ueber drei Samplerates und fuenf Blockgroessen, 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur passive_probe zu; speichern-laden-speichern bytegleich. | `(nicht gefahren)` | [STILLGELEGT] seit 2026-08-28 (S9b/SONDE-007c): das Ziel NakamaSuna ist stillgelegt - Suna ist in Nakama Probeeq aufgegangen (design/abnahmen/2026-08-28-suna-stilllegung-vorgezogen.md). Weder gebaut noch gefahren; die Zeile bleibt sichtbar, damit die Kanonzahl nicht still sinkt. | - | - |
| A16 | Nakama Probeeq (NkAc): heute ebenfalls Passthrough bitgleich (die EQ-DSP kommt in P6), 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur active_probe zu; speichern-laden-speichern bytegleich. Seit der T3-Nacharbeit 29.08.2026 (G1 §4.2) faehrt dasselbe Bein die Gate-7-Kette AM GEBAUTEN BUNDLE Nakama Probeeq - Klasse active_probe, denn die Sondenschale traegt genau EINE Klasse je Uebersetzung: ein sonst gueltiger Stand mit measurement_position=post_fader_contribution kommt ueber setStateInformation read-only zurueck, die Instanz bleibt neutral, die Originalbytes gehen unveraendert an den Host zurueck und ein erneutes Laden waescht nichts; auch eine bereits klassifizierte Instanz faellt beim Nachreichen desselben Standes auf neutral zurueck; Gegenprobe mit insert laedt normal. Nachtrag Runde 2 (Prueferbefund P1): dass Riegel 1 KLASSENUNABHAENGIG sperrt, misst dasselbe Bein zusaetzlich direkt an der oeffentlichen positionErlaubt fuer alle vier Klassen - ohne diese vier Zeilen bliebe die urspruengliche passive_probe-Regression hier unbemerkt, weil kein Ziel mehr NAKAMA_SONDE_PASSIV baut (S9b/SONDE-007c). Die vollstaendige 16er-Matrix samt Bundlevertraegen misst B2. | `eq-copilot\build\plugin\EqCopProbeeqNullTest_artefacts\Release\EqCopProbeeqNullTest.exe` | [OK] Exit 0 | 0,06 s | [A16](roh/NAK-96-8a1ea8a.md#a16) |
| B1 | Bundle-Identitaet (CIDs, JUCE_VST3_CAN_REPLACE_VST2=0) eingefroren. | `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | [OK] Exit 0 | 0,07 s | [B1](roh/NAK-96-8a1ea8a.md#b1) |
| B2 | State-Schema 2: Roundtrip bytegleich, Schema-1-Migration rein und golden, unbekanntes Major read-only mit Originalbytes, Duplicate erkennbar (gleiche instance_id, verschiedene runtime_nonce) und aufloesbar, Host-Dirty; Parametertabelle deckungsgleich mit dem Vertrag; RFC-8785-state_hash bytegleich zu Python und Rust. | `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | [OK] Exit 0 | 0,34 s | [B2](roh/NAK-96-8a1ea8a.md#b2) |
| B3 | Hostkontext (Anwesenheit, Parameterpunkte, Buslatenz) wird gemessen, nicht geraten; Quellhash-Gate des JUCE-Patches gruen. | `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | [OK] Exit 0 | 0,06 s | [B3](roh/NAK-96-8a1ea8a.md#b3) |
| B3b | Termin-B-Messgeraet: Passthrough bitgleich, Sprung-/Automations-/Latenzmessung inkl. Fehlalarm-Riegel, Bericht-Rueckweg, 0 Allokationen. | `eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe` | [OK] Exit 0 | 0,10 s | [B3b](roh/NAK-96-8a1ea8a.md#b3b) |
| B3c | v3-Vertrag: C++ klassifiziert den Fixture-Korpus wie das Manifest (Urteil UND Verletzungsmenge), Bandgitter und Quantisierung bitgleich. | `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | [OK] Exit 0 | 0,20 s | [B3c](roh/NAK-96-8a1ea8a.md#b3c) |
| B4 | StampedAudioQueue und Ein-Block-Quarantaene: Ganzblockaufnahme bitgleich ueber jeden Ringumlauf, Ueberlauf BEIDER Ringe verwirft den ganzen Block und nie eine Teilmenge, Oversize ueber der Slotkapazitaet faellt fuer die Analyse und laesst Audio unberuehrt, Flush (numSamples 0) ist kein Verlust, Mono dupliziert L, Mehrfach-Tap-Layout traegt; Quarantaene versiegelt erst mit bewiesener Fortsetzung, Seek und Transportkante verwerfen genau EINEN Block, stehende Projektzeit (FL-Teilpuffer) ist kein Bruch, Projektzeit-Ueberlauf und negative Zeit sind behandelt; Worker-Publikation folgt monotonen 50-/250-ms-Deadlines statt Batchzahl, holt nach Pausen nicht auf und laesst wartende Reset-/Frame-Aufrufer vor; verdrahtet: Passthrough bitgleich ueber 18 Blockgroessen von 1 bis 16384, 0 Samples Latenz, kein Tail, 0 Allokationen im Audiothread ueber 4000 Bloecke wechselnder Groesse mit Transportkanten, und die Engine bekommt den Strom bis auf den Block in Quarantaene. | `eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe` | [OK] Exit 0 | 3,30 s | [B4](roh/NAK-96-8a1ea8a.md#b4) |
| B9 | Fixed-memory Loudness (§48.1): der LoudnessAccumulator deckt sich mit der ausgebauten Vektorrechnung innerhalb ±0,1 LU (Entwurf §49) ueber konstante, rampende, zufaellige und einstuendige Korpora sowie ueber Stille unter dem absoluten Gate; Kurz-LUFS ist BITGLEICH; ein adversarialer Sweep legt 1000 Bloecke in den Grenzbin des relativen Gates und die selbstgemeldete Schranke unsicherheitLu() deckt jeden Lauf; eine Million Zellen laufen mit 0 Allokationen durch, waehrend die Gegenprobe (alte Rechnung) allozert; NaN/Inf-Zellen sind gezaehlt statt still als 0 verbucht und l_j == -70,0 exakt bleibt wertgleich; ueber dem Feingitter traegt ein OBERBAND aus Bins von 1 LU bis ueber lautheit(DBL_MAX) - mit ZWEI Pegeln darin (Korpus des T2-Pruefers), einem adversarialen Sweep im Oberband-Grenzbin, beiden Richtungen der Naht zwischen den Aufloesungen und der Gegenprobe, dass kein Block durch das Raster faellt. | `eq-copilot\build\plugin\EqCopLoudnessGoldenTest_artefacts\Release\EqCopLoudnessGoldenTest.exe` | [OK] Exit 0 | 0,15 s | [B9](roh/NAK-96-8a1ea8a.md#b9) |
| B5 | FeatureEngine v2 haelt Zeit-, Validity-, Event- und Bandvertraege: Bandgitter und alle 61 Quantisierungsvektoren bitgleich zum v3-Vertrag, Bitmap LSB-first mit Fuellbits 0, FFT gegen Parseval und einen Sinus auf der Binmitte, K-Gewichtung ueber 20 Hz..20 kHz unter 0,1 dB an der BS.1770-Referenzkette; Drop/Seek(laufend UND gestoppt)/Loop-Wrap/moeglicher Straddle/Transportkante/Sampleratewechsel/Neuanlauf/Beweislagewechsel trennen JEDES offene Fenster - auch den K-Filterzustand, bitgleich gemessen - waehrend FL-Teilstuecke mit stehender Projektzeit lokal weiterlaufen, aber kein unbewiesenes Projektintervall oder FFT-Event erzeugen; Frame-Stempel umfassen den echten ueberlappenden FFT-Support und bleiben an int64/u32-Grenzen darstellbar; Drop zaehlt als Segment, alles andere als Epoche; alle sieben Erzeuger-Stempelregeln fallen einzeln und mit eigener Nummer; kein spektraler Fluss ueberbrueckt eine Grenze, der Ereignisring ist fest gedeckelt; LUFS-S trifft die analytisch gerechnete Erwartung unter 0,1 LU; die Nyquist-Kappe greift bei 22,05 kHz wirklich; verdrahtet: alle sieben Gueltigkeitsbits kommen ueber die Hostbruecke durch, der Playhead-Rueckfallweg nachweislich nur zwei. | `eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe` | [OK] Exit 0 | 2,41 s | [B5](roh/NAK-96-8a1ea8a.md#b5) |
| B6 | Aktiver DSP-Kern liefert die eingefrorene Referenzantwort. | `eq-copilot\build\plugin\EqCopDspGoldenTest_artefacts\Release\EqCopDspGoldenTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B7 | Apply/Revert ist transaktional - kein halber Zustand ueberlebt. | `eq-copilot\build\plugin\EqCopTransactionTest_artefacts\Release\EqCopTransactionTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B10 | v3-Envelope in C++ klassifiziert den Envelope-Korpus wie das Manifest (Urteil UND Verstossmenge, alle 14 Regeln mit Negativfixture); CRC32C trifft die RFC-3720-Vektoren, P0/P1 tragen CRC exakt 0, P2 die Pflichtsumme ueber genau die Payloadbytes; 40 000 Zufallspuffer bringen den Pruefer nie aus dem Tritt und 7671 angenommene EINBIT-Mutanten gueltiger Frames halten jede Kopfregel (reiner Zufall wird praktisch immer abgewiesen - die Invariante braucht deshalb die Mutanten, sonst spraeche sie ueber eine leere Menge), 3000 gekippte P2-Bits fallen einzeln, byteweise Zustellung liefert dieselben 40 Frames und ein kaputter Frame beendet den Strom; Pipetoken trifft das Golden aus §48.3 samt SHA-256- und RFC-4648-Vektoren; P0 verwirft nichts und meldet den 65. Eintrag, P1 koalesziert an der Position und haelt Ereignisse fuer den Reconnect vor, die P2-Schleuse ersetzt den aeltesten ungesendeten Frame, uebergibt 100 000 Frames mit 0 Allokationen (mit Gegenprobe am selben Zaehler) und liefert unter Flut keinen zerrissenen Frame; verdrahtet: Control koppelt Telemetry ueber link_id + challenge, ein ungekoppelter Telemetry-Connect wird geschlossen, der Client verbindet nach Serverneustart von selbst wieder, ein kaputter Envelope vom Server schliesst die Verbindung, und ein P0-Ueberlauf WAEHREND einer stehenden Verbindung schliesst sie ebenfalls statt still zu kuerzen. | `eq-copilot\build\plugin\EqCopIpcTest_artefacts\Release\EqCopIpcTest.exe` | [ROT] Exit 1 | 33,28 s | [B10](roh/NAK-96-8a1ea8a.md#b10) |
| B8 | Lifecycle-Klassifikation §53.5: unclassified beim Laden und audio-neutral; Schema-1 sensor\|pre\|post -> legacy (immer passiv), hub bzw. bestaetigter Schema-2-Main-State -> main; ein Scannerlauf klassifiziert nicht; read-only nimmt die Klassifikation zurueck; Brokerstart nur fuer main mit offenem Editor; die Sondenbundles bleiben bis gueltigem State neutral und werden nie main. | `eq-copilot\build\plugin\EqCopLebenslaufTest_artefacts\Release\EqCopLebenslaufTest.exe` | [OK] Exit 0 | 0,11 s | [B8](roh/NAK-96-8a1ea8a.md#b8) |


---

## Kanon-Lauf - NAK-96 Abschlusslauf 2

**Lauf:** 2026-08-29 14:16 | **Runner:** `tools/beweise.ps1` | **Urteil:** GRUEN - 32/32 Kanon-Laeufe bestanden | 2 geplante Pruefung(en) noch nicht gebaut | 1 stillgelegte(s) Bein(e), siehe Uebersicht | **Exitcode:** 0 | **Rohausgabe:** [roh/NAK-96-d993894.md](roh/NAK-96-d993894.md)

### Kopf - woran gemessen wurde

| Feld | Wert |
|---|---|
| Zeitpunkt | 2026-08-29 14:16:55 +02:00 |
| Rechner | SCHUBBINATOR200 \| Windows 10.0.26200.0 |
| Zweig | master |
| Commit | d993894 NAK-96: Abschlusslauf 1 auf 8a1ea8a - Aufteilung greift, B10 EqCopIpcTest faellt lastabhaengig (nicht NAK-96) |
| Commit (voll) | d993894d7a27f35e41c52519345810382e28a22d |
| Arbeitsbaum | sauber |
| JUCE gepinnt | 8.0.9 |
| JUCE auf Platte | 8.0.9-dirty |
| FL Studio | FL Studio 2025 25.2.5.5319 \| FL Studio 2026 26.1.4.5589 |
| cargo | cargo 1.93.1 (083ac5135 2025-12-15) |
| rustc | rustc 1.93.1 (01f6ddf75 2026-02-11) |
| PowerShell | 7.6.5 |
| cmake | cmake version 3.31.6-msvc6 |

### Uebersicht

| # | Behauptung | Befehl | Ergebnis | Dauer | Rohausgabe |
|---|---|---|---|---|---|
| A1 | Passthrough ist bitgleich; 0 Samples Latenz, 0 Tail; NaN/Inf werden gezaehlt, aber nicht veraendert. | `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | [OK] Exit 0 | 0,07 s | [A1](roh/NAK-96-d993894.md#a1) |
| A2 | AnalyseEngine deckt sich mit der eingefrorenen Offline-Referenz (Fixture-SHA-256 als Determinismus-Riegel). | `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | [OK] Exit 0 | 11,77 s | [A2](roh/NAK-96-d993894.md#a2) |
| A3 | Hoer-Markierung bleibt verriegelt: Render/Freilauf bitgleich, Analyse-Abgriff sitzt vor der Faerbung, und der U10-Term ist gedeckt - OHNE Playhead faerbt bei sonst voller Erlaubnis kein Sample (T11, mit Gegenprobe bei laufendem Transport). | `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | [OK] Exit 0 | 6,34 s | [A3](roh/NAK-96-d993894.md#a3) |
| A4 | Broker-Vertragstests gruen (Framing, Protokoll, Bindung, Aggregat, Server). | `cargo test --manifest-path broker/Cargo.toml --color never` | [OK] Exit 0 | 3,14 s | [A4](roh/NAK-96-d993894.md#a4) |
| A4b | C++-PipeClient: stop/reconnect und ganze Frames sind zeitlich begrenzt, SQOS ist Identification, Peerbytes sind UTF-8/NUL-sauber, ACK-Sequenzen streng und u64-Zaehler wire-sicher. | `eq-copilot\build\plugin\EqCopPipeClientTest_artefacts\Release\EqCopPipeClientTest.exe` | [OK] Exit 0 | 1,08 s | [A4b](roh/NAK-96-d993894.md#a4b) |
| A5 | Referenzbein (jsonschema, draft 2020-12): Schema haelt die Engine-Teilmenge ein, Textriegel deckt jede gemessene Kante, jedes Fixture wird wie im Manifest klassifiziert, jede Definition hat ein Negativfixture. | `py -3.13 tools\eq-copilot\pruefe_v3_vertrag.py --abdeckung` | [OK] Exit 0 | 1,82 s | [A5](roh/NAK-96-d993894.md#a5) |
| A6 | Beide Bandgitter sind bytegleich zur Neuerzeugung; 221 Baender, 64 Gruppen als exakte Partition. | `py -3.13 tools\eq-copilot\erzeuge_bandgitter.py --pruefen` | [OK] Exit 0 | 0,17 s | [A6](roh/NAK-96-d993894.md#a6) |
| A7 | Quantisierungsvertrag bytegleich zur Neuerzeugung; Rundung, Saettigung und Nichtendliches als Testvektoren. | `py -3.13 tools\eq-copilot\erzeuge_quantisierung.py --pruefen` | [OK] Exit 0 | 0,17 s | [A7](roh/NAK-96-d993894.md#a7) |
| A8 | Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_v3_fixtures.py --pruefen` | [OK] Exit 0 | 0,23 s | [A8](roh/NAK-96-d993894.md#a8) |
| A9 | Codegen-Drift ist 0: die Neugenerierung aus dem .fbs ist bytegleich zum committeten C++- und Rust-Code; flatc, C++-Header und Rust-Crate tragen dieselbe gepinnte Version; jedes Tabellenfeld traegt eine explizite Feld-ID. | `py -3.13 tools\eq-copilot\pruefe_flatc_drift.py` | [OK] Exit 0 | 0,35 s | [A9](roh/NAK-96-d993894.md#a9) |
| A10 | Binaerer Fixture-Korpus und sein MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_fb_fixtures.py --pruefen` | [OK] Exit 0 | 1,12 s | [A10](roh/NAK-96-d993894.md#a10) |
| A11 | Die fuenf v2-Vertraege (ipc v2, measurement v1, report v1, snapshot v3, aggregat v1) sind gueltiges JSON und gueltige JSON-Schemas; ihre $id-Familie ist eingefroren. | `py -3.13 tools\eq-copilot\pruefe_v2_schemas.py` | [OK] Exit 0 | 0,33 s | [A11](roh/NAK-96-d993894.md#a11) |
| A12 | Parameterbestand (109 IDs, §53.8) haelt den Vertrag; RFC-8785-Zahlenvektoren tragen den RFC-Text und werden von rfc8785 bestaetigt; State-Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung. | `py -3.13 tools\eq-copilot\erzeuge_state_fixtures.py --pruefen` | [OK] Exit 0 | 0,30 s | [A12](roh/NAK-96-d993894.md#a12) |
| A13 | Capabilityreport FL: die zehn Bits aus §53.6 entsprechen der v3-Vertragsform und stehen so, wie die Rohdaten der Termine A und B sie tragen; jedes supported hat einen Termin, jedes unsupported seinen festen Fallback. | `py -3.13 tools\eq-copilot\pruefe_host_capabilities.py` | [OK] Exit 0 | 0,24 s | [A13](roh/NAK-96-d993894.md#a13) |
| A14 | Der gemeinsame Kern traegt keine Bundle-Identitaet: NakamaKern.lib enthaelt keinen eingefrorenen Identitaetswert (Namen, Viercodes, CIDs roh und COM-vertauscht) und genau seine eigenen Uebersetzungseinheiten, kein JUCE-Modulobjekt; die Gegenprobe findet dieselben Werte im gebauten EQ-Copilot-Bundle. | `py -3.13 tools\eq-copilot\pruefe_kern_identitaetsfrei.py` | [OK] Exit 0 | 0,59 s | [A14](roh/NAK-96-d993894.md#a14) |
| A17 | Installer-Manifest: keine zweite Identitaet (kein Viercode, keine Class-ID), jeder Quellpfad ist der aus Ziel + Identitaetsdatei nachgerechnete BUNDLE-ORDNER, `art` ist eine geschlossene Menge, jedes Ziel genau einmal, Broker aus dem Crate-Namen, Zielverzeichnisse geschuetzt, Signaturzeile behauptet keine Pruefung ohne Mittel, Rueckweg samt NAK-41-Riegel und hash_art vollstaendig; jede Regel faellt an einem verdorbenen Manifest; Ordner-Hash v1 bytegleich in Python und PowerShell, Nicht-ASCII bricht beide Haelften ab. | `py -3.13 tools\eq-copilot\pruefe_installer_manifest.py` | [OK] Exit 0 | 1,09 s | [A17](roh/NAK-96-d993894.md#a17) |
| A18 | Gegenpfad installieren<->Rueckweg gefahren (Sandbox, nichts installiert): Erstinstallation traegt moduleinfo.json, ein Tausch ersetzt statt zu mischen, -Pruefen sieht den ganzen Ordner, der Rueckweg stellt den Vorzustand bytegleich her und laesst KEIN leeres .vst3-Gehaeuse stehen, selbst angelegte Ordner verschwinden nur leer, der NAK-41-Riegel greift und zerstoert seine eigene Wiederholungsquelle nicht, eine dritte Artefaktsorte bricht ab. | `py -3.13 tools\eq-copilot\pruefe_installer_gegenpfad.py` | [OK] Exit 0 | 49,83 s | [A18](roh/NAK-96-d993894.md#a18) |
| A19 | BandGridZahlen.h ist bytegleich aus den zwei eingefrorenen Gitterfixturen erzeugt; die 64 Live-Gruppen partitionieren die 221 Feinbaender lueckenlos und ueberschneidungsfrei, und die groben Kanten sind bitgleiche Kopien feiner Kanten (kein zweites Filterbank-Gitter). | `py -3.13 tools\eq-copilot\erzeuge_bandgitter_header.py --pruefen` | [OK] Exit 0 | 0,18 s | [A19](roh/NAK-96-d993894.md#a19) |
| A20 | Envelope-Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei; jede der 14 Envelope-Regeln hat mindestens ein Negativfixture. | `py -3.13 tools\eq-copilot\erzeuge_envelope_fixtures.py --pruefen` | [OK] Exit 0 | 0,45 s | [A20](roh/NAK-96-d993894.md#a20) |
| A21 | v3-Envelope unter Zufall: 20 000 Zufallspuffer bringen den Pruefer nie aus dem Tritt und JEDER angenommene Frame erfuellt jede Kopfregel; 3000 gekippte P2-Payloadbits fallen einzeln an der CRC; feindliche Laengen (0, 15, >Grenze, 0xFFFFFFFF) und die u32-Grenze von 16+payload_len loesen keine Allokation aus; 300 Runden Fragmentierung (byteweise und in Zufallshaeppchen) liefern exakt dieselben Frames; ein kaputter Frame beendet den Strom statt zu resynchronisieren; die Ratengrenze haelt unter Flut. | `cargo test --manifest-path broker/Cargo.toml --test transport_fuzz --color never` | [OK] Exit 0 | 0,39 s | [A21](roh/NAK-96-d993894.md#a21) |
| A22 | Ende-zu-Ende ueber die PROBE-Pipe, zwei Sprachen ein Draht: 32 echte C++-Sondenpaare koppeln sich am echten Rust-Listener (32 Control + 32 Telemetry, jede Telemetrieverbindung ueber link_id + challenge + gleiche runtime_nonce), fluten P2 bis die Schleuse mit Cap 2 nachweislich ersetzt, und WAEHRENDDESSEN geht kein einziger P0-Frame verloren; die P0-Antwortlatenz bleibt unter der Schranke. Keine Verbindung wird wegen Envelope, Rate oder P0-Ueberlauf geschlossen. | `py -3.13 tools\eq-copilot\pruefe_ipc_last.py` | [OK] Exit 0 | 10,42 s | [A22](roh/NAK-96-d993894.md#a22) |
| A15 | STILLGELEGT - mass bis 28.08.2026 Nakama Suna (NkPr): Passthrough bitgleich ueber drei Samplerates und fuenf Blockgroessen, 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur passive_probe zu; speichern-laden-speichern bytegleich. | `(nicht gefahren)` | [STILLGELEGT] seit 2026-08-28 (S9b/SONDE-007c): das Ziel NakamaSuna ist stillgelegt - Suna ist in Nakama Probeeq aufgegangen (design/abnahmen/2026-08-28-suna-stilllegung-vorgezogen.md). Weder gebaut noch gefahren; die Zeile bleibt sichtbar, damit die Kanonzahl nicht still sinkt. | - | - |
| A16 | Nakama Probeeq (NkAc): heute ebenfalls Passthrough bitgleich (die EQ-DSP kommt in P6), 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur active_probe zu; speichern-laden-speichern bytegleich. Seit der T3-Nacharbeit 29.08.2026 (G1 §4.2) faehrt dasselbe Bein die Gate-7-Kette AM GEBAUTEN BUNDLE Nakama Probeeq - Klasse active_probe, denn die Sondenschale traegt genau EINE Klasse je Uebersetzung: ein sonst gueltiger Stand mit measurement_position=post_fader_contribution kommt ueber setStateInformation read-only zurueck, die Instanz bleibt neutral, die Originalbytes gehen unveraendert an den Host zurueck und ein erneutes Laden waescht nichts; auch eine bereits klassifizierte Instanz faellt beim Nachreichen desselben Standes auf neutral zurueck; Gegenprobe mit insert laedt normal. Nachtrag Runde 2 (Prueferbefund P1): dass Riegel 1 KLASSENUNABHAENGIG sperrt, misst dasselbe Bein zusaetzlich direkt an der oeffentlichen positionErlaubt fuer alle vier Klassen - ohne diese vier Zeilen bliebe die urspruengliche passive_probe-Regression hier unbemerkt, weil kein Ziel mehr NAKAMA_SONDE_PASSIV baut (S9b/SONDE-007c). Die vollstaendige 16er-Matrix samt Bundlevertraegen misst B2. | `eq-copilot\build\plugin\EqCopProbeeqNullTest_artefacts\Release\EqCopProbeeqNullTest.exe` | [OK] Exit 0 | 0,05 s | [A16](roh/NAK-96-d993894.md#a16) |
| B1 | Bundle-Identitaet (CIDs, JUCE_VST3_CAN_REPLACE_VST2=0) eingefroren. | `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | [OK] Exit 0 | 0,07 s | [B1](roh/NAK-96-d993894.md#b1) |
| B2 | State-Schema 2: Roundtrip bytegleich, Schema-1-Migration rein und golden, unbekanntes Major read-only mit Originalbytes, Duplicate erkennbar (gleiche instance_id, verschiedene runtime_nonce) und aufloesbar, Host-Dirty; Parametertabelle deckungsgleich mit dem Vertrag; RFC-8785-state_hash bytegleich zu Python und Rust. | `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | [OK] Exit 0 | 0,34 s | [B2](roh/NAK-96-d993894.md#b2) |
| B3 | Hostkontext (Anwesenheit, Parameterpunkte, Buslatenz) wird gemessen, nicht geraten; Quellhash-Gate des JUCE-Patches gruen. | `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | [OK] Exit 0 | 0,05 s | [B3](roh/NAK-96-d993894.md#b3) |
| B3b | Termin-B-Messgeraet: Passthrough bitgleich, Sprung-/Automations-/Latenzmessung inkl. Fehlalarm-Riegel, Bericht-Rueckweg, 0 Allokationen. | `eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe` | [OK] Exit 0 | 0,09 s | [B3b](roh/NAK-96-d993894.md#b3b) |
| B3c | v3-Vertrag: C++ klassifiziert den Fixture-Korpus wie das Manifest (Urteil UND Verletzungsmenge), Bandgitter und Quantisierung bitgleich. | `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | [OK] Exit 0 | 0,22 s | [B3c](roh/NAK-96-d993894.md#b3c) |
| B4 | StampedAudioQueue und Ein-Block-Quarantaene: Ganzblockaufnahme bitgleich ueber jeden Ringumlauf, Ueberlauf BEIDER Ringe verwirft den ganzen Block und nie eine Teilmenge, Oversize ueber der Slotkapazitaet faellt fuer die Analyse und laesst Audio unberuehrt, Flush (numSamples 0) ist kein Verlust, Mono dupliziert L, Mehrfach-Tap-Layout traegt; Quarantaene versiegelt erst mit bewiesener Fortsetzung, Seek und Transportkante verwerfen genau EINEN Block, stehende Projektzeit (FL-Teilpuffer) ist kein Bruch, Projektzeit-Ueberlauf und negative Zeit sind behandelt; Worker-Publikation folgt monotonen 50-/250-ms-Deadlines statt Batchzahl, holt nach Pausen nicht auf und laesst wartende Reset-/Frame-Aufrufer vor; verdrahtet: Passthrough bitgleich ueber 18 Blockgroessen von 1 bis 16384, 0 Samples Latenz, kein Tail, 0 Allokationen im Audiothread ueber 4000 Bloecke wechselnder Groesse mit Transportkanten, und die Engine bekommt den Strom bis auf den Block in Quarantaene. | `eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe` | [OK] Exit 0 | 3,39 s | [B4](roh/NAK-96-d993894.md#b4) |
| B9 | Fixed-memory Loudness (§48.1): der LoudnessAccumulator deckt sich mit der ausgebauten Vektorrechnung innerhalb ±0,1 LU (Entwurf §49) ueber konstante, rampende, zufaellige und einstuendige Korpora sowie ueber Stille unter dem absoluten Gate; Kurz-LUFS ist BITGLEICH; ein adversarialer Sweep legt 1000 Bloecke in den Grenzbin des relativen Gates und die selbstgemeldete Schranke unsicherheitLu() deckt jeden Lauf; eine Million Zellen laufen mit 0 Allokationen durch, waehrend die Gegenprobe (alte Rechnung) allozert; NaN/Inf-Zellen sind gezaehlt statt still als 0 verbucht und l_j == -70,0 exakt bleibt wertgleich; ueber dem Feingitter traegt ein OBERBAND aus Bins von 1 LU bis ueber lautheit(DBL_MAX) - mit ZWEI Pegeln darin (Korpus des T2-Pruefers), einem adversarialen Sweep im Oberband-Grenzbin, beiden Richtungen der Naht zwischen den Aufloesungen und der Gegenprobe, dass kein Block durch das Raster faellt. | `eq-copilot\build\plugin\EqCopLoudnessGoldenTest_artefacts\Release\EqCopLoudnessGoldenTest.exe` | [OK] Exit 0 | 0,15 s | [B9](roh/NAK-96-d993894.md#b9) |
| B5 | FeatureEngine v2 haelt Zeit-, Validity-, Event- und Bandvertraege: Bandgitter und alle 61 Quantisierungsvektoren bitgleich zum v3-Vertrag, Bitmap LSB-first mit Fuellbits 0, FFT gegen Parseval und einen Sinus auf der Binmitte, K-Gewichtung ueber 20 Hz..20 kHz unter 0,1 dB an der BS.1770-Referenzkette; Drop/Seek(laufend UND gestoppt)/Loop-Wrap/moeglicher Straddle/Transportkante/Sampleratewechsel/Neuanlauf/Beweislagewechsel trennen JEDES offene Fenster - auch den K-Filterzustand, bitgleich gemessen - waehrend FL-Teilstuecke mit stehender Projektzeit lokal weiterlaufen, aber kein unbewiesenes Projektintervall oder FFT-Event erzeugen; Frame-Stempel umfassen den echten ueberlappenden FFT-Support und bleiben an int64/u32-Grenzen darstellbar; Drop zaehlt als Segment, alles andere als Epoche; alle sieben Erzeuger-Stempelregeln fallen einzeln und mit eigener Nummer; kein spektraler Fluss ueberbrueckt eine Grenze, der Ereignisring ist fest gedeckelt; LUFS-S trifft die analytisch gerechnete Erwartung unter 0,1 LU; die Nyquist-Kappe greift bei 22,05 kHz wirklich; verdrahtet: alle sieben Gueltigkeitsbits kommen ueber die Hostbruecke durch, der Playhead-Rueckfallweg nachweislich nur zwei. | `eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe` | [OK] Exit 0 | 2,38 s | [B5](roh/NAK-96-d993894.md#b5) |
| B6 | Aktiver DSP-Kern liefert die eingefrorene Referenzantwort. | `eq-copilot\build\plugin\EqCopDspGoldenTest_artefacts\Release\EqCopDspGoldenTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B7 | Apply/Revert ist transaktional - kein halber Zustand ueberlebt. | `eq-copilot\build\plugin\EqCopTransactionTest_artefacts\Release\EqCopTransactionTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B10 | v3-Envelope in C++ klassifiziert den Envelope-Korpus wie das Manifest (Urteil UND Verstossmenge, alle 14 Regeln mit Negativfixture); CRC32C trifft die RFC-3720-Vektoren, P0/P1 tragen CRC exakt 0, P2 die Pflichtsumme ueber genau die Payloadbytes; 40 000 Zufallspuffer bringen den Pruefer nie aus dem Tritt und 7671 angenommene EINBIT-Mutanten gueltiger Frames halten jede Kopfregel (reiner Zufall wird praktisch immer abgewiesen - die Invariante braucht deshalb die Mutanten, sonst spraeche sie ueber eine leere Menge), 3000 gekippte P2-Bits fallen einzeln, byteweise Zustellung liefert dieselben 40 Frames und ein kaputter Frame beendet den Strom; Pipetoken trifft das Golden aus §48.3 samt SHA-256- und RFC-4648-Vektoren; P0 verwirft nichts und meldet den 65. Eintrag, P1 koalesziert an der Position und haelt Ereignisse fuer den Reconnect vor, die P2-Schleuse ersetzt den aeltesten ungesendeten Frame, uebergibt 100 000 Frames mit 0 Allokationen (mit Gegenprobe am selben Zaehler) und liefert unter Flut keinen zerrissenen Frame; verdrahtet: Control koppelt Telemetry ueber link_id + challenge, ein ungekoppelter Telemetry-Connect wird geschlossen, der Client verbindet nach Serverneustart von selbst wieder, ein kaputter Envelope vom Server schliesst die Verbindung, und ein P0-Ueberlauf WAEHREND einer stehenden Verbindung schliesst sie ebenfalls statt still zu kuerzen. | `eq-copilot\build\plugin\EqCopIpcTest_artefacts\Release\EqCopIpcTest.exe` | [OK] Exit 0 | 32,55 s | [B10](roh/NAK-96-d993894.md#b10) |
| B8 | Lifecycle-Klassifikation §53.5: unclassified beim Laden und audio-neutral; Schema-1 sensor\|pre\|post -> legacy (immer passiv), hub bzw. bestaetigter Schema-2-Main-State -> main; ein Scannerlauf klassifiziert nicht; read-only nimmt die Klassifikation zurueck; Brokerstart nur fuer main mit offenem Editor; die Sondenbundles bleiben bis gueltigem State neutral und werden nie main. | `eq-copilot\build\plugin\EqCopLebenslaufTest_artefacts\Release\EqCopLebenslaufTest.exe` | [OK] Exit 0 | 0,09 s | [B8](roh/NAK-96-d993894.md#b8) |

---

## 8. NAK-97 — planstand.py (Folge-Worker, Basis f501704)

| Feld | Wert |
|---|---|
| Ticket | `NAK-97` (beide Punkte) |
| Quelle | `docs/offene-punkte.md`, Zeile `| NAK-97 |` — **wörtlich**, nicht meine Zusammenfassung |
| Basis-SHA | `f50170486603566cfbbd59a8b7ad95e59e6b87a1` (Zweig `master`) |
| Datum | 2026-08-29 |
| Werkzeugeingriff | ausschließlich `tools/plan/planstand.py` |
| Commit | `da1b04e9562a37d60de5e6a12ac393c3c9292085` |

NAK-97 ist der Rest von NAK-96 in einer Datei, die der NAK-96-Auftrag nicht
abdeckte — beide Punkte sind dieselben zwei Klassen (lesender git-Aufruf ohne
`--no-optional-locks`; ein Urteilstext des Runners, den die Gegenseite nicht
liest). Es wird zusammen mit NAK-96 geprüft.

**Diese Datei bleibt lesbar:** Abschnitt 8 hängt hinter den Kanon-Abschnitten,
zitiert aber **keine** `**Lauf:**`-Zeile im Klartext — genau die Falle aus
**§5.1**. Gemessen in §8.4: `kanon_lesen()` liefert für diese Datei unverändert
`Kanon 32/32 grün`. (`docs/beweise/NAK-96.md` ist selbst kein Beleg eines
Planschritts; die Regel gilt trotzdem.)

<a id="b81"></a>
### 8.1 · Auftrag, wörtlich aus `docs/offene-punkte.md`

> **Zwei Reste derselben Klassen wie NAK-96, in einer Datei, die der Auftrag
> nicht abdeckte: `tools/plan/planstand.py`.** (1) `planstand.py:95` startet git
> ohne `--no-optional-locks`; die Aufrufer sind `git log -1` und
> **`git status --porcelain`** (Zeilen 247–254) — genau der Index-Refresh,
> dessen liegengebliebene `.git/index.lock` NAK-96 (2) ausgelöst hat. Das
> Skript läuft aus `tools/hooks/planstand.sh` und damit auf denselben
> Lastpfaden wie das Cockpit. Fix: derselbe Schalter als erstes Argument vor
> `-C`. (2) Die Regex `KANON` (`planstand.py:66`) verlangt `(\d+)/(\d+)\s*Kanon`;
> der Runner schreibt diese Form nur im GRÜN-Fall
> (`GRUEN - 30/32 Kanon-Laeufe bestanden`). Im ROT-Fall lautet der Text
> `ROT - 1 von 32 Kanon-Laeufen fehlgeschlagen` — **ohne Schrägstrich**, die
> Regex greift nicht, und der Planstand zeigt gar keine Kanon-Zahl statt einer
> roten. […] Die Regex nennt `ROT` als Alternative, kann sie aber nie treffen;
> entweder der Runner schreibt im ROT-Fall dieselbe `n/m`-Form, oder die Regex
> fängt beide Wortlaute.

<a id="b82"></a>
### 8.2 · Umgesetzte Entscheidungen und die eigenen dazu

| # | Entscheidung | Umsetzung |
|---|---|---|
| A | Beide Wortlaute liefern **dieselbe** Semantik `bestanden/gesamt`: GRÜN `n/m` → `Kanon n/m grün`; ROT `k von m fehlgeschlagen` → `Kanon (m−k)/m ROT`. Letzter Treffer in der Datei gewinnt. | `KANON` ist eine Alternation mit benannten Gruppen (`gruen_gut`/`gruen_ges` bzw. `rot_fehl`/`rot_ges`); `kanon_lesen()` nimmt den letzten Treffer aus `finditer` und rechnet im ROT-Zweig `ges − fehl`. Gemessen (a)–(c2) in §8.3. |
| B | Kommentar über `KANON` nennt beide Wortlaute wörtlich samt Quelle. | Der Kommentar zitiert die beiden `$urteil`-Zeilen aus `tools/beweise.ps1` **Zeile 856** (GRÜN) und **845** (ROT); die Zeilennummern sind am heutigen Skript nachgeschlagen, nicht aus dem Gedächtnis. |
| C | Kein Kanon-Lauf. | Kein Kanon-Bein fährt `tools/plan/planstand.py` — der Kanon prüft Plugin, Broker und Schemata. Ein Lauf hätte über diese Änderung nichts ausgesagt; der Beweis sind die Proben §8.3–§8.6. |

**Eigene technische Entscheidungen innerhalb der Grenze**

1. **Die beiden übrigen Urteilstexte bleiben ohne Treffer.** `tools/beweise.ps1`
   schreibt vier Urteile, nicht zwei: neben GRÜN (856) und ROT (845) auch
   `UNVOLLSTAENDIG - n gruen, k Voraussetzung(en) fehlen` (849) und
   `NICHT BEGLAUBIGT - n/m gruen, aber Pruefbinaries sind aelter als die
   Quellen` (853). Beide bleiben absichtlich unerfasst: dort hat der Runner die
   Beglaubigung gerade **verweigert**, und eine Zeile `Kanon n/m grün` würde ein
   bestandenes Ergebnis behaupten, das es nicht gibt. Fail-closed lieber keine
   Zahl — dieselbe Regel, nach der der Kopf des Skripts eine fehlende
   Urteilsmarke lieber untertreibt. Gemessen als (f1)/(f2) in §8.3.
2. **`?` statt einer negativen Zahl.** Steht in einem Manifest mehr
   Fehlschläge als Läufe, ist die Datei verdorben; `ges − fehl` wäre negativ.
   `kanon_lesen()` schreibt dann `Kanon ?/m ROT` statt einer erfundenen Zahl.
   Der Runner kann das nicht schreiben — deshalb steht die Behauptung nur so
   weit, wie Probe (g) in §8.3 sie misst.
3. **Die ROT-Alternative in `n/m`-Form fällt weg.** Die alte Regex hätte auch
   ein `ROT - 3/5 Kanon` getroffen; die neue verlangt im ROT-Zweig `k von m`.
   Gemessen, dass der Korpus keine solche Zeile enthält (§8.4), und der Runner
   schreibt sie nicht. Eine dritte Alternative wäre eine Behauptung ohne
   Messung.

<a id="b83"></a>
### 8.3 · Regex-Probe: beide Wortlaute, echte Manifeste, Grenzfälle

Die Probe **importiert** `KANON` und `kanon_lesen` aus `tools/plan/planstand.py`
und bildet sie nicht nach — eine nachgebaute Regex würde sich selbst prüfen.
Die Zeilennummer im Kopf ist gemessen, nicht abgeschrieben.

**Befehl:** `py -3.13 -c "$CODE" <tmp>`, `$CODE` = das Probenskript (lädt
`tools/plan/planstand.py` über `importlib`) · **Datum:** 2026-08-29

```text
Regex KANON aus tools/plan/planstand.py, Zeile 85 (gemessen):
    Urteil:\*\*\s*(?:GRUEN\s*[-—]+\s*(?P<gruen_gut>\d+)/(?P<gruen_ges>\d+)\s*Kanon|ROT\s*[-—]+\s*(?P<rot_fehl>\d+)\s+von\s+(?P<rot_ges>\d+)\s*Kanon)

(a) GRUEN-Wortlaut des Runners (beweise.ps1 Zeile 856)
    Urteilstext: GRUEN - 30/32 Kanon-Laeufe bestanden
    kanon_lesen: 'Kanon 30/32 grün'
    erwartet   : 'Kanon 30/32 grün'   OK
(b) ROT-Wortlaut des Runners (beweise.ps1 Zeile 845)
    Urteilstext: ROT - 1 von 32 Kanon-Laeufen fehlgeschlagen
    kanon_lesen: 'Kanon 31/32 ROT'
    erwartet   : 'Kanon 31/32 ROT'   OK
(c) ROT nach GRUEN in derselben Datei - juengster Lauf gewinnt
    kanon_lesen: 'Kanon 28/32 ROT'
    erwartet   : 'Kanon 28/32 ROT'   OK
(c2) GRUEN nach ROT - Gegenrichtung
    kanon_lesen: 'Kanon 32/32 grün'
    erwartet   : 'Kanon 32/32 grün'   OK
(d) echtes Manifest docs/beweise/NAK-96.md
    kanon_lesen: 'Kanon 32/32 grün'
    erwartet   : 'Kanon 32/32 grün'   OK
(e) echtes Manifest docs/beweise/lauf-2026-08-27-0322.md
    kanon_lesen: 'Kanon 28/29 ROT'
    erwartet   : 'Kanon 28/29 ROT'   OK
(f1) UNVOLLSTAENDIG (beweise.ps1 Zeile 849) - absichtlich kein Treffer
    Urteilstext: UNVOLLSTAENDIG - 30 gruen, 2 Voraussetzung(en) fehlen
    kanon_lesen: ''
    erwartet   : ''   OK
(f2) NICHT BEGLAUBIGT (beweise.ps1 Zeile 853) - absichtlich kein Treffer
    Urteilstext: NICHT BEGLAUBIGT - 32/32 gruen, aber Pruefbinaries sind aelter als die Quellen
    kanon_lesen: ''
    erwartet   : ''   OK
(g) verdorbenes Manifest (mehr Fehlschlaege als Laeufe)
    Urteilstext: ROT - 5 von 2 Kanon-Laeufen fehlgeschlagen
    kanon_lesen: 'Kanon ?/2 ROT'
    erwartet   : 'Kanon ?/2 ROT'   OK

9 von 9 Proben OK
```

**Gemessen:** (a) GRÜN unverändert · (b) ROT wird jetzt gelesen · (c) ein ROT
**nach** einem GRÜN gewinnt, (c2) ein GRÜN **nach** einem ROT ebenfalls — damit
ist „letzter Treffer gewinnt" gemessen und nicht „ROT gewinnt immer" ·
(d) das echte Manifest dieses Tickets bleibt bei `Kanon 32/32 grün` ·
(e) `docs/beweise/lauf-2026-08-27-0322.md` ist das bestehende Manifest mit
rotem Lauf (gesucht mit `grep -l` über `docs/beweise/*.md`) und liefert jetzt
`Kanon 28/29 ROT` statt gar nichts · (f1)/(f2) die nicht beglaubigenden
Urteilstexte bleiben ohne Zahl · (g) ein verdorbenes Manifest liefert `?`,
keine negative Zahl.

<a id="b84"></a>
### 8.4 · Korpusprobe: alter gegen neuen Leser über alle Manifeste

Beide Fassungen von `kanon_lesen()` laufen im selben Prozess über
`docs/beweise/**/*.md` — der alte Leser kommt aus `git show HEAD:...`, nicht aus
einer Kopie von Hand.

**Befehl:** `py -3.13 -c "$CODE" <tmp>/planstand_alt.py` · **Datum:** 2026-08-29

```text
docs/beweise/G1.md                             alt='Kanon 28/28 grün'   neu='Kanon 28/28 grün'     
docs/beweise/KONTEXT-INVENTUR-2026-08-21.md    alt='Kanon 15/15 grün'   neu='Kanon 15/15 grün'     
docs/beweise/lauf-2026-08-27-0012.md           alt='Kanon 28/28 grün'   neu='Kanon 28/28 grün'     
docs/beweise/lauf-2026-08-27-0322.md           alt=''                   neu='Kanon 28/29 ROT'     <-- GEAENDERT
docs/beweise/lauf-2026-08-27-0326.md           alt='Kanon 29/29 grün'   neu='Kanon 29/29 grün'     
docs/beweise/NAK-96.md                         alt='Kanon 32/32 grün'   neu='Kanon 32/32 grün'     
docs/beweise/roh/NAK-96-2271df5-dirty-2.md     alt=''                   neu='Kanon 31/32 ROT'     <-- GEAENDERT
docs/beweise/roh/NAK-96-2271df5-dirty.md       alt=''                   neu='Kanon 31/32 ROT'     <-- GEAENDERT
docs/beweise/roh/NAK-96-8a1ea8a.md             alt=''                   neu='Kanon 31/32 ROT'     <-- GEAENDERT
docs/beweise/roh/NAK-96-bruchprobe-2271df5-dirty.md alt=''                   neu='Kanon 31/32 ROT'     <-- GEAENDERT
docs/beweise/roh/NAK-96-bruchprobe.md          alt=''                   neu='Kanon 31/32 ROT'     <-- GEAENDERT
docs/beweise/roh/NAK-96-d993894.md             alt='Kanon 32/32 grün'   neu='Kanon 32/32 grün'     
docs/beweise/S0-basislinie.md                  alt='Kanon 4/4 grün'     neu='Kanon 4/4 grün'       
docs/beweise/SONDE-001-002.md                  alt='Kanon 5/5 grün'     neu='Kanon 5/5 grün'       
docs/beweise/SONDE-003.md                      alt='Kanon 6/6 grün'     neu='Kanon 6/6 grün'       
docs/beweise/SONDE-003b.md                     alt='Kanon 7/7 grün'     neu='Kanon 7/7 grün'       
docs/beweise/SONDE-004.md                      alt='Kanon 18/18 grün'   neu='Kanon 18/18 grün'     
docs/beweise/SONDE-004a.md                     alt='Kanon 5/5 grün'     neu='Kanon 5/5 grün'       
docs/beweise/SONDE-005a.md                     alt='Kanon 29/29 grün'   neu='Kanon 29/29 grün'     
docs/beweise/SONDE-005b.md                     alt='Kanon 29/29 grün'   neu='Kanon 29/29 grün'     
docs/beweise/SONDE-006.md                      alt='Kanon 29/29 grün'   neu='Kanon 29/29 grün'     
docs/beweise/SONDE-007a.md                     alt='Kanon 29/29 grün'   neu='Kanon 29/29 grün'     
docs/beweise/SONDE-007b.md                     alt='Kanon 32/32 grün'   neu='Kanon 32/32 grün'     
docs/beweise/SONDE-007c.md                     alt='Kanon 28/28 grün'   neu='Kanon 28/28 grün'     
docs/beweise/SONDE-008.md                      alt='Kanon 28/28 grün'   neu='Kanon 28/28 grün'     
docs/beweise/SONDE-009.md                      alt='Kanon 28/28 grün'   neu='Kanon 28/28 grün'     
docs/beweise/SONDE-010.md                      alt='Kanon 32/32 grün'   neu='Kanon 32/32 grün'     

27 Manifeste mit Kanon-Zahl, davon 6 geaendert
```

**Gemessen:** 27 Manifeste tragen eine Kanon-Zahl. Sechs ändern sich, und
**jede** dieser sechs Änderungen geht von „gar keine Zahl" zu einer roten Zahl.
Keine einzige grüne Zahl bewegt sich, keine Zahl verschwindet. Keines der sechs
ist Beleg eines Planschritts (`lauf-*` und `roh/*`) — deshalb steht das
erzeugte `docs/PLAN-STAND.md` unverändert (§8.6).

<a id="b85"></a>
### 8.5 · Bruchprobe (Prüfliste E): der Fix einmal zurückgenommen

`tools/plan/planstand.py` wurde per `git show HEAD:tools/plan/planstand.py` auf
den Stand `f501704` zurückgesetzt — der echte alte Stand, kein nachgebauter —,
dieselbe Probe gefahren, danach aus der Sicherung zurückgeholt.

**Rohausgabe des Rots:**

```text
Regex KANON aus tools/plan/planstand.py, Zeile 66 (gemessen):
    Urteil:\*\*\s*(GRUEN|ROT)\s*[-—]+\s*(\d+)/(\d+)\s*Kanon

(a) GRUEN-Wortlaut des Runners (beweise.ps1 Zeile 856)
    Urteilstext: GRUEN - 30/32 Kanon-Laeufe bestanden
    kanon_lesen: 'Kanon 30/32 grün'
    erwartet   : 'Kanon 30/32 grün'   OK
(b) ROT-Wortlaut des Runners (beweise.ps1 Zeile 845)
    Urteilstext: ROT - 1 von 32 Kanon-Laeufen fehlgeschlagen
    kanon_lesen: ''
    erwartet   : 'Kanon 31/32 ROT'   FEHLER
(c) ROT nach GRUEN in derselben Datei - juengster Lauf gewinnt
    kanon_lesen: 'Kanon 32/32 grün'
    erwartet   : 'Kanon 28/32 ROT'   FEHLER
(c2) GRUEN nach ROT - Gegenrichtung
    kanon_lesen: 'Kanon 32/32 grün'
    erwartet   : 'Kanon 32/32 grün'   OK
(d) echtes Manifest docs/beweise/NAK-96.md
    kanon_lesen: 'Kanon 32/32 grün'
    erwartet   : 'Kanon 32/32 grün'   OK
(e) echtes Manifest docs/beweise/lauf-2026-08-27-0322.md
    kanon_lesen: ''
    erwartet   : 'Kanon 28/29 ROT'   FEHLER
(f1) UNVOLLSTAENDIG (beweise.ps1 Zeile 849) - absichtlich kein Treffer
    Urteilstext: UNVOLLSTAENDIG - 30 gruen, 2 Voraussetzung(en) fehlen
    kanon_lesen: ''
    erwartet   : ''   OK
(f2) NICHT BEGLAUBIGT (beweise.ps1 Zeile 853) - absichtlich kein Treffer
    Urteilstext: NICHT BEGLAUBIGT - 32/32 gruen, aber Pruefbinaries sind aelter als die Quellen
    kanon_lesen: ''
    erwartet   : ''   OK
(g) verdorbenes Manifest (mehr Fehlschlaege als Laeufe)
    Urteilstext: ROT - 5 von 2 Kanon-Laeufen fehlgeschlagen
    kanon_lesen: ''
    erwartet   : 'Kanon ?/2 ROT'   FEHLER

5 von 9 Proben OK
```

**Gemessen:** ohne den Fix fallen genau (b), (c), (e) und (g). Der schärfste
Fall ist **(c)**: eine Datei, deren jüngster Lauf ROT ist, meldet dem Planstand
`Kanon 32/32 grün` — die alte Regex überspringt den roten Lauf und findet den
älteren grünen. Nicht „keine Zahl", sondern eine **falsche grüne**. (a), (c2),
(d), (f1) und (f2) bleiben auch gebrochen grün; sie sind Regressionswachen für
das unveränderte GRÜN-Verhalten, kein Beleg für den Fix.

Rücknahme des Bruchs gemessen: derselbe Probelauf steht in §8.3 wieder auf
9/9, und der Umfang des committeten Fixes ist genau der, der vor dem Bruch
gesichert wurde — keine Zeile mehr, keine weniger:

```text
$ git --no-optional-locks show --stat --oneline da1b04e -- tools/plan/planstand.py
da1b04e NAK-97: planstand.py liest beide Kanon-Wortlaute, git-Aufruf mit --no-optional-locks

 tools/plan/planstand.py | 59 ++++++++++++++++++++++++++++++++++++++++++++-----
 1 file changed, 54 insertions(+), 5 deletions(-)
```

<a id="b86"></a>
### 8.6 · git-Probe: der Schalter steht im Aufruf, das Blatt bleibt gleich

**Befehl und Rohausgabe** · **Datum:** 2026-08-29

```text
$ grep -n "subprocess.run" tools/plan/planstand.py     # einziger git-Aufruf des Skripts
130:        return subprocess.run(["git", "--no-optional-locks", "-C", str(WURZEL), *args],

$ grep -n "no-optional-locks" tools/plan/planstand.py tools/beweise.ps1 tools/dirigent/cockpit.ps1 | grep -v "#"     # nur echte Aufrufstellen
tools/plan/planstand.py:121:    `--no-optional-locks` steht VOR `-C`, weil git globale Schalter vor dem
tools/plan/planstand.py:130:        return subprocess.run(["git", "--no-optional-locks", "-C", str(WURZEL), *args],
tools/beweise.ps1:36:    Lesende git-Aufrufe laufen mit --no-optional-locks (NAK-96): sonst frischt
tools/beweise.ps1:182:    $r = Fuehre-Aus -Datei 'git' -Argumente (@('--no-optional-locks', '-C', $Wurzel) + $Argumente)
tools/beweise.ps1:575:    $r = Fuehre-Aus -Datei 'git' -Argumente @('--no-optional-locks', '-C', $juceQuelle, 'describe', '--tags', '--always', '--dirty')
tools/dirigent/cockpit.ps1:180:        return Invoke-TextProcess $gitPath "--no-optional-locks -C $(Quote-ProcessArgument $script:RepoRoot) $Arguments" $TimeoutSeconds

$ py -3.13 tools/plan/planstand.py
geschrieben: docs\PLAN-STAND.md (14 abgenommen, 5 gebaut, 38 gesamt, aus f501704)
exit=0

$ git --no-optional-locks diff --stat -- docs/PLAN-STAND.md
 docs/PLAN-STAND.md | 6 +++---
 1 file changed, 3 insertions(+), 3 deletions(-)
```

`planstand.py` hat **genau einen** git-Aufruf (`subprocess.run`, Zeile 130); er
bedient beide Aufrufer aus §8.1 (`git log -1` und `git status --porcelain`).
Der Schalter steht als erstes Argument vor `-C`, wortgleich zu
`tools/beweise.ps1:182` (`Git-Wert`) und `tools/dirigent/cockpit.ps1:180`
(`Invoke-GitText`).

Vergleich des **erzeugten** Blattes vor und nach dem Umbau — beide Läufe auf
demselben Quellstand `f501704`:

```text
$ diff -u plan-stand-vorher.md plan-stand-nachher.md   # erzeugt vor / nach dem Umbau
--- C:/Users/phili/.claude/jobs/7b187706/tmp/plan-stand-vorher.md	2026-08-29 14:28:05.165154300 +0200
+++ C:/Users/phili/.claude/jobs/7b187706/tmp/plan-stand-nachher.md	2026-08-29 14:32:36.734564900 +0200
@@ -11,6 +11,9 @@
 
 **Stand:** 2026-08-29 · Quellstand `f501704` · **14 von 38 abgenommen** · 5 gebaut · 19 offen
 
+> ⚠️ Gerechnet aus dem Arbeitsbaum: unter `docs/plan/`, `docs/beweise/`
+> oder `tools/plan/` liegen Änderungen, die noch nicht in `f501704` sind.
+
 `███████████████▓▓▓▓▓░░░░░░░░░░░░░░░░░░░░` 37 % abgenommen · 50 % gebaut
 
 **Als Nächstes:** **Nacharbeit an G1** — der Prüfer hat einen Befund offen gelassen (docs/beweise/G1.md).
diff-exit=1
```

**Gemessen:** identisch bis auf die Arbeitsbaum-Warnung, die genau deshalb
erscheint, weil `tools/plan/` beim zweiten Lauf uncommittet geändert war —
also der Beweis, dass die Warnung greift, und keine Abweichung im Inhalt. Der
`quellstand` ist in beiden Läufen `f501704`. Keine neue Warnung auf stderr,
Exit 0.

**Was hier NICHT behauptet wird:** dass es keine liegengebliebene
`.git/index.lock` mehr gibt — das ist an einem einzelnen Lauf nicht messbar
(dieselbe Grenze wie NAK-96 §5.4). Behauptet und gemessen ist nur: der einzige
lesende git-Aufruf dieses Skripts trägt den Schalter an der Stelle, an der git
ihn liest.

<a id="b87"></a>
### 8.7 · Prüfliste `tools/dirigent/pruefliste.md` — wo gemessen

| Klasse | Zutreffend? | Wo gemessen / warum nicht |
|---|---|---|
| **A** Rückstau und Prioritätsklassen | nein | Kein Puffer, keine Queue, keine Prioritätsklasse; geändert sind eine Regex, eine Lesefunktion und ein Argumentvektor. |
| **B** Lebenszyklus | nein | Kein Verbinden/Trennen, kein Start/Stop, kein Thread. |
| **C** Verträge und Längen | teilweise | Der Vertrag ist der Wortlaut der `**Lauf:**`-Zeile. Beide Formen sind gegen die **Quelle** geprüft (`tools/beweise.ps1` Zeile 845/856), nicht gegen eine Kopie; Grenzfälle (verdorbene Zahl, nicht beglaubigende Urteile) in **§8.3** (f1)(f2)(g). |
| **D** Bau- und Prüfriegel | ja | Der Leser bleibt fail-closed: was der Runner nicht beglaubigt hat, bekommt weiterhin **keine** Zahl (§8.3 f1/f2) — er wird durch den Fix nicht redseliger, sondern nur an der einen Stelle ehrlich, wo bisher ein rotes Urteil unsichtbar war. Kein Kanon-Bein fährt diese Datei (§8.2 C), deshalb kein Lauf; die Beglaubigungslogik in `tools/beweise.ps1` ist unangetastet. |
| **E** Behauptung ≤ Messung | ja | Jede Behauptung steht neben ihrer Rohausgabe (§8.3–§8.6). Der Fix wurde einmal absichtlich zurückgenommen, Rohausgabe des Rots liegt bei (**§8.5**). Über den Index-Lock wird nichts behauptet, was ein Lauf nicht zeigt (**§8.6**). Die Zeilennummer der Regex misst die Probe selbst; die Kanon-Zahlen kommen aus dem Korpus, nicht aus einer anderen Datei (**§8.4**). |
| **F** Änderungssatz | ja | lesen↔schreiben im selben Commit: der Runner **schreibt** beide Urteilstexte, `planstand.py` **liest** ab jetzt beide — vorher war das Paar halb. Regex, Lesefunktion und der Kommentar, der die Quelle benennt, liegen in einem Commit (`da1b04e`); Manifest und Register folgen im zweiten. |

<a id="b88"></a>
### 8.8 · Was nicht erledigt ist

- **Kein Kanon-Lauf gefahren** — begründet in §8.2 C. Die Urteilsmarke für
  NAK-96 und NAK-97 setzt der Prüfer.
- **Befund außerhalb der Ticketgrenze:** `KANON` steht durch den geforderten
  Kommentar (Entscheid B) nicht mehr auf Zeile 66, sondern auf **Zeile 85**.
  Der Kommentar in `tools/beweise.ps1:907` nennt „`tools/plan/planstand.py`
  (KANON, Zeile 66)"; das Symbol `KANON` stimmt weiter, die Zahl nicht mehr.
  `tools/beweise.ps1` ist ausdrücklich außerhalb dieses Auftrags, deshalb
  unangetastet und als Nachtrag im Register festgehalten. Die „Zeile 66" in
  **§3** und **§5.1** dieses Manifests bleibt richtig: sie beschreibt den Stand,
  an dem NAK-96 gemessen hat.
