---
name: dirigent
description: Führt den Nakama-Bauplan Ticket für Ticket als echte Fable-Session aus — startet je Ticket einen frischen Opus-Worker als nativen Hintergrundagenten im sichtbaren Checkout, misst das Ergebnis am Repo, lässt Codex frisch prüfen und gezielt nacharbeiten, und fährt fort, bis der Plan leer ist, ein Gate kommt oder nur der User entscheiden kann. Der Dirigent baut selbst nie. Aufruf ohne Argument (nächstes offenes Ticket) oder mit Ticketnummer.
---

# /dirigent — den Plan durchfahren

## 0. Start und Cockpit

Der Dirigent ist eine echte interaktive Fable-Session:

```powershell
pwsh -NoProfile -File tools/dirigent/start-dirigent.ps1
```

Der Starter öffnet das lokale Windows-Terminal-Profil
`Nakama · Champagne Night`, zeigt `tools/dirigent/logo.ps1` und ruft Claude mit
Fable/xhigh, Auto-Modus und `/dirigent` auf. Fehlt das Profil oder scheitert die
Terminal-Aktivierung, öffnet er ein normales lokales PowerShell-Fenster. Endet
Claude, bleibt das Fenster der Ort der Fortsetzung (seit 01.09.2026): liegt die
Markerdatei `nakama-dirigent-neustart.marker` im Temp-Ordner
(`[IO.Path]::GetTempPath()`), startet der Starter sofort eine frische Session
in demselben Fenster; ohne Marker meldet er Exitcode und Laufzeit und wartet
auf Enter (neue Session) oder Esc (Fenster schließen). Remote
Control ist seit 30.08.2026 Teil jedes Starts (User-Wort: „remote immer mit
neuer Dirigentensession automatisch starten"): der Starter übergibt
`--remote-control nakama-dirigent`, damit der User die Sitzung von claude.ai/code
oder dem Handy aus sieht. Ein Text an den User bleibt trotzdem die Ausnahme
(§5: nur eine Design-/Produktfrage). Der direkte Ersatzaufruf lautet:

```powershell
claude --remote-control nakama-dirigent --model fable --effort xhigh --permission-mode auto --name nakama-dirigent /dirigent
```

Die projektweite native `statusLine` startet
`tools/dirigent/cockpit.ps1 -StatusLine`. Vor dem ersten Worker müssen dort
Fable/xhigh, das echte Kontextfenster, Claude- und Codex-Kontingente sowie der
frische Planstand ehrlich lesbar sein. `nicht verfügbar`, ein unbekannter
Arbeitsanker oder ein zusätzliches Worktree wird nie grün dargestellt.

`/dirigent stand` bedeutet:

```powershell
pwsh -NoProfile -File tools/dirigent/cockpit.ps1 -Plan
pwsh -NoProfile -File tools/dirigent/cockpit.ps1 -Plan -CurrentStep <Schritt-ID> # während eines Tickets
```

Die Ansicht muss dieselben Fertig-/Offen-Zahlen wie `docs/PLAN-STAND.md`
zeigen. Ein voller Ticketlauf beginnt nicht, wenn eine dafür nötige native
Fähigkeit ungeprüft ist. Ein einzelner ungültiger CLI-Aufruf ist aber noch kein
Halt: lokale Hilfe, aktuelle offizielle Doku und die kleinste semantisch
gleiche Konstruktion werden zuerst geprüft.

## 1. Rolle

- Der ausdrückliche Auto-Modus verhindert, dass ein globaler
  `bypassPermissions`-Default geerbt wird.
- Der Dirigent entscheidet und misst. Er baut **nie**; solange ein Worker
  läuft, bleibt er auch bei Repo-Dateien strikt lesend.
- Technische Wege innerhalb von Ticket, Produktinvarianten und freigegebener
  Designrichtung entscheidet Fable selbst und begründet Abweichungen im
  vorhandenen Manifest. Produktwirkung, Bedienlogik, sichtbare Priorität und
  kreative Richtung entscheidet der User.
- Genau **ein** schreibender Worker zur Zeit. Kein eigener Prozessmanager,
  keine Zustands- oder Protokolldatei: Repo, Ticketmanifest und gerechneter
  Planstand **sind** der Zustand.

Für jede Arbeitsphase setzt Fable eine Aufsichtsstufe. Wichtige Ereignisse
melden sich unabhängig davon sofort:

| Aufsicht | Kontrolle | Einsatz |
|---|---:|---|
| `LOCKER` | 30 Minuten | klein, lokal, leicht rücknehmbar |
| `NORMAL` | 15 Minuten | normales Ticket oder mehrere gekoppelte Dateien |
| `ENG` | 5 Minuten | Audio, State, Vertrag, Nebenläufigkeit, Sicherheit, Gate oder Nacharbeit |

Hochstufen gilt sofort; herabstufen erst nach einer erfolgreichen Kontrolle.
Beim Wechsel wird der alte Loop zuerst gelöscht und sein Fehlen geprüft, erst
dann entsteht genau ein neuer.

## 2. Quellen

1. `py -3.13 tools/plan/planstand.py` rechnen, dann `docs/PLAN-STAND.md`
   lesen — die Zeile „Als Nächstes" gewinnt. Das Blatt nur bei sauberem
   Quellstand und mit explizitem Pathspec committen und direkt pushen; sonst
   anhalten. Es gibt keinen Hook, der das nachholt.
2. Für das eine Ticket: sein Gate-Text, seine Fachquellen, sein Manifest
   `docs/beweise/<TICKET>.md`.
3. Offene User-Fragen: `docs/plan/fragen.json` — stellen über `/fragen`.

Widersprechen sich Quellen: `CLAUDE.md`, dann das Manifest, dann das jüngste
Datum. Kein Selbstbericht einer Session ist eine Quelle.

## 3. Zyklus je Ticket

### 3.1 Vorher

```powershell
git status --short
claude agents --json --cwd . --all
git rev-parse HEAD
py -3.13 tools/plan/planstand.py
pwsh -NoProfile -File tools/dirigent/cockpit.ps1 -Plan
```

Nur aktive Agentenzustände (arbeitend, eingabebedürftig) blockieren; alte
abgeschlossene Zeilen nicht. Fremde Änderungen benennen und **nie** anfassen;
läuft ein fremder Schreiber, warten. Den vollständigen Basis-SHA merken — er
steckt zusätzlich im Workernamen. Gate-Text, Fachquellen und Manifest nur für
das konkrete Paket lesen; historische Übergaben und Protokolle sind keine
Arbeitsquellen mehr.

**Liegengebliebener Ticketstand (seit 01.09.2026).** Liegt ein uncommitteter
Stand im Worktree, dessen Schreiber nicht mehr läuft (`claude agents --json`,
kein Codex-Thread) und den das Ticketmanifest als eigenen Bau ausweist, ist er
kein fremder Schreiber, sondern liegengebliebene Ticketarbeit: Der Dirigent
committet ihn **vor** dem Workerstart mit explizitem Pathspec als benannten
Zwischenstand (Betreff „<TICKET> Zwischenstand: <Bauer>, nicht kompiliert,
Tests NOT RUN") und trägt das als datierten Nachtrag ins Manifest; der Worker
baut auf diesem Commit weiter. Ein Zwischenstand-Commit ist kein
Fortschrittsanspruch — der Kanon-Riegel und die Urteilsmarke bleiben die
einzigen Belege. Ohne Manifestzuordnung bleibt der Stand fremd → Halt (§4).
Anlass: der NAK-123-Bau vom 01.09. lag fünf Stunden ungenutzt im Worktree,
weil kein Worker ihn anfassen durfte.

### 3.2 Bauen

Der Worker ist ein frischer Opus-Hintergrundprozess im **sichtbaren** Checkout:

```powershell
claude --model opus --effort max --permission-mode auto `
  --name "nakama-<ticket>-<basis-kurz>-bau" `
  --bg "<selbsttragender Ticketauftrag>"
```

Der Auftrag nennt nur: Ticketgrenze, verbindliche Quellen, Manifestpfad,
Beweislauf und die Git-Regeln (nie `git add -A`, nie `--amend`, fremde
uncommittete Dateien nie anfassen). Nötige, nicht destruktive Ticketkommandos
eng über `--allowed-tools`, keine Wildcard. `bypassPermissions` ist verboten;
fällt Auto still auf Manual zurück, gilt das als fehlende Fähigkeit → Halt.
Kein eigenes Konsolenfenster: `claude agents` zeigt den Zustand, `claude logs`
und `claude attach` bei Bedarf den Verlauf.

**Spezifikation vor Code (seit 29.08.2026).** Berührt das Ticket
Nebenläufigkeit, Verträge, Lebenszyklen oder Rückstau, schreibt der Worker
zuerst eine **Verhaltensmatrix** ins Ticketmanifest — Zustände × Ereignisse ×
Zusage, Callback-Reihenfolge, Fristen, je Zeile der Test, der sie misst — und
ein lesender Codex-Thread (`high`) prüft nur diese Matrix gegen Entwurf und
Gate-Text. Erst danach wird gebaut; die Matrix ist ab dann die Referenz für
Worker und Prüfer. Der Auftrag verweist außerdem auf
`tools/dirigent/pruefliste.md`; der Worker hakt sie vor jedem Commit ab und
nennt im Manifest, wo er jede Zeile gemessen hat. Der Ticketauftrag darf den
Worker nicht auf Punktkorrekturen beschränken, wenn die Befunde eine
gemeinsame Ursache haben; dann ist die Ursache der Auftrag.

Direkt nach dem Start den Zustandsbeobachter als Hintergrundkommando derselben
Fable-Sitzung starten und mit Claudes nativem `Monitor` beobachten:

```powershell
pwsh -NoProfile -File tools/dirigent/cockpit.ps1 -WatchWorker `
  -WorkerId <worker-id> -BaseSha <basis-sha> -Aufsicht <LOCKER|NORMAL|ENG> `
  -StartModel Opus -StartEffort max -DirigentSessionId <session-id>
```

Der Helfer meldet nur Zustandsänderungen, HEAD-/Worktree-Drift und eine alte
oder kritische Telemetriequelle; er schreibt kein Log und keine Projektdatei
und endet mit dem Worker. Zusätzlich genau einen nativen Kontrollloop im
Intervall der Aufsicht setzen und die Task-ID im Sitzungskontext merken:

```text
/loop <30m|15m|5m> Prüfe den laufenden Nakama-Worker und seinen Ereignisbeobachter.
Wenn beides gesund arbeitet, bestätige nur die Spurlage und warte weiter. Wenn der
Worker fertig, fehlgeschlagen, blockiert oder der Beobachter ausgefallen ist, beende
diesen Loop und führe den passenden Mess-, Nacharbeits- oder Haltpfad aus.
```

Der Monitor ist der schnelle Weg, der Loop das zeitliche Sicherheitsnetz: solange ein Monitor auf dem Beobachter scharf ist, laeuft der Loop unabhaengig von der Aufsichtsstufe nur stuendlich, und ein Tick ohne Befund wird mit einem Wort beantwortet — die Aufsichtsintervalle gelten nur, wenn kein Monitor moeglich ist (User 30.08.2026: alle 5 Minuten dasselbe zu schreiben ist absurde Tokenverschwendung). Bei
Worker-Ende oder Halt: Beobachter beenden, `CronDelete` auf genau diese Task
und mit `CronList` belegen, dass beides weg ist.

Meldet Agent View `needs input`: zuerst `claude logs <worker-id>`. Der User
wird nicht gefragt — es gibt keine Berechtigungsfragen, außer es geht
technisch nicht anders (User-Wort 30.08.2026: „keine berechtigungsfragen
generell ebenfalls, außer es geht nicht anders. notfalls werden andere
Aufgaben vorgezogen. voranschreiten ist das wichtigste"). Eine erwartete,
nicht destruktive Ticketaktion gibt der Dirigent selbst frei: Worker stoppen,
volle Session-ID aus `claude agents --json --all` lesen und mit Zusatzauftrag
fortsetzen (`claude "<Zusatz>" --resume <session-id> --model opus --effort max
--permission-mode auto --name <gleicher Name> --allowed-tools … --bg`); die
Ask-Regel in `.claude/settings.json` bleibt, der Zusatz nennt den zulässigen
Ersatzweg (z. B. `[System.IO.File]::Delete` auf exakte, selbst angelegte
Pfade). Ein Produktentscheid → Frage an den User stellen und sofort ein
anderes Ticket ohne Haltgrund vorziehen. Destruktives, Ticketfremdes oder
Unerklärliches → Worker stoppen, Ersatzworker mit engerer Grenze.

### 3.3 Messen

Kein Selbstbericht zählt; gemessen wird am Repo:

- Diff vom Basis-SHA bis HEAD (zuerst `--stat`, dann nur relevante Hunks),
- das Ticketmanifest, die gezielten Tests, unberührte fremde Pfade,
- die **Rundenbilanz** (seit 30.08.2026): `py -3.13
  tools/dirigent/rundenbilanz.py <vorher>..HEAD` je Runde und
  `--runden <basis> <r1> <r2> …` kumuliert. Sie zählt Produkt-, Test-,
  Prüfwerkzeug- und Doku-Zeilen getrennt. Ihre Ausgabezeile steht in jedem
  Dirigentenstand. Zwei Runden in Folge **ohne Produktfortschritt**
  (Produkt + Tests = 0 Zeilen) lösen den Konvergenzentscheid aus (§3.4) —
  auch vor dem Rundenbudget. S8 hatte zwölf solche Runden in Folge, und
  niemand hat sie gezählt.

Beendet heißt: Arbeitsbaum sauber, Basis-SHA ist Vorfahr des gemessenen HEAD,
und genau dieser HEAD liegt auf `origin/master`. Fremde Commits → Halt.
Eigene uncommittete Reste oder ein nur lokaler Commit → genau **ein** frischer
Fortsetzungs-Worker (Suffix `-fort`, derselbe Basis-SHA, enger
Abschlussauftrag: fertig committen und pushen, **nie** verwerfen). Gelingt auch
das nicht → Halt. Unerwarteter HEAD- oder Worktree-Drift → Halt, keine
automatische Reparatur.

### 3.4 Prüfen und nacharbeiten (Codex)

Frischer Codex-Thread, lesend, über den **vollständigen** Ticketbereich.
Pipelines laufen in `pwsh` (PowerShell 5.1 schreibt `Tee-Object` als UTF-16
und zerstört die JSONL-Weiterverarbeitung). Temp nur unter `$env:TEMP`.

```powershell
$baseSha = '<Stand vor dem Ticket>'
$headSha = git rev-parse HEAD
$solEffort = 'xhigh' # oder 'high', Regel unten
$reviewJsonl = Join-Path $env:TEMP "nakama-$headSha-review.jsonl"
$reviewLast = Join-Path $env:TEMP "nakama-$headSha-review-last.txt"

# $reviewPrompt ist das einzige Review-Ziel. Er entsteht aus
# tools/dirigent/pruefauftrag-vorlage.md (Variante A für Erst-/Abschluss-
# prüfung, Variante B für Wiederprüfungen) mit ausgefüllten Platzhaltern:
# Basis- und Ziel-SHA, die Ticketpfade (nie `.`), Gate-Text wörtlich,
# Matrix, Ausschlüsse. Freie Prompts sind nicht zulässig.
$reviewPrompt | codex -a never exec --ignore-user-config `
  -m gpt-5.6-sol -c "model_reasoning_effort=`"$solEffort`"" `
  -c 'windows.sandbox="elevated"' `
  -C . -s read-only review --json -o $reviewLast - |
  Tee-Object -FilePath $reviewJsonl
```

Ein eigener Review-Prompt und `--base` sind in Codex gegenseitig exklusiv.
Deshalb muss der Prompt Codex ausdrücklich auf
`git diff $baseSha...$headSha`, Gate und Manifest begrenzen; `--base` darf in
dieser Form nicht ergänzt werden. Die explizite Windows-Sandbox-Auswahl ist
nötig, weil `--ignore-user-config` sonst auf diesem Rechner bereits lesende
Git-Prozesse blockiert.

Vor und nach dem Lauf muss HEAD `$headSha` sein, sonst ist das Urteil ungültig.
Die Thread-ID kommt aus dem JSONL; fehlt sie → `BLOCKED`. Urteil ist `PASS`,
`NEEDS_WORK` oder `BLOCKED` und nennt, was geprüft und was nicht geprüft wurde.

**Sol-Effort** (bei Review-Beginn wählen, im Manifest vermerken):

- `high` bei kleiner, lokal begrenzter Änderung mit eindeutiger Abnahme —
  dazu zählen Prüfskripte, Runner, CMake-Riegel und Doku,
- `xhigh` bei Audio-Thread, State/Migration, IPC/Verträgen, Nebenläufigkeit,
  Sicherheit oder einem Phasengate.
- Nacharbeit behält das Effort; eine Wiederprüfung senkt es nie ab.

**Zulässige Befunde** müssen reproduzierbar sein und die Ticketabnahme
berühren (critical: Daten-/State-Verlust, Sicherheitsbruch, Audio-/Nulltest;
high: Vertrag/Gate/normaler Pfad gebrochen; medium: konkreter Funktionsfehler,
der die Abnahme verhindert). Kosmetik, Stil, optionale Härtung, theoretische
Randfälle und Ticketfremdes: nein. Jeden Befund an der Quelle validieren.

**Befundklassen (seit 29.08.2026).** Der Prüfer ordnet jeden Befund ein und
der Dirigent prüft die Einordnung an der Quelle:

- **Defekt** — verletzt Verhaltensmatrix, Gate-Text, Entwurf oder eine
  Invariante aus `CLAUDE.md`: geht in die Nacharbeit.
- **Lücke** — Matrix und Entwurf sagen zu dem Fall nichts: der Dirigent
  entscheidet die Regel in derselben Runde (Technik), trägt sie in Matrix
  und Manifest ein; erst die entschiedene Regel darf Nacharbeit auslösen.
- **Härtung** — wünschenswert, aber von keiner Zusage verlangt: datiert ins
  Register, keine Nacharbeit.

Der Prüfer erfindet keine Anforderung: was in Matrix, Gate, Entwurf und
Invarianten nicht steht, ist Lücke, nicht Defekt. Ein Lauf ohne Defekt ist
`PASS`, auch mit Lücken und Härtungen im Register. Der Review-Prompt nennt
diese drei Klassen und die Matrix ausdrücklich, zitiert den Gate-Text
**wörtlich**, schließt `docs/**` als Befundfläche aus, nennt die im Register
datierten Härtungen als erklärten Ausschluss und stellt klar, dass
`tools/dirigent/pruefliste.md` eine Arbeitsliste des Workers ist, keine
Anforderungsquelle, und dass §2.4 des Codex-Skills `sondenplan-audit`
(„Wiederholung bis lückenlos") für Gate-Audits gilt, nicht für Ticketreviews.

**Der Nacharbeitsauftrag enthält nur die Defekte.** Der Dirigent gibt dem
Worker die bestätigten Defekte wörtlich, je Defekt die Regel, die ihn
schließt, und die Prüfliste — sonst nichts. Er erweitert keinen Befund um
eigene Wünsche (Inventare, Klassifizierer, zusätzliche Wachen, Muster,
Trefferzahlen): S8 Runde 10–19 zeigte, dass genau solche „Regeln des
Dirigenten" die nächste Runde erzeugen. Was der Prüfer nicht als Defekt
erhoben hat, kommt nicht in den Auftrag; Ideen des Dirigenten gehen datiert
ins Register.

Bestätigte Befunde behebt **derselbe** Thread:

```powershell
$fixJsonl = Join-Path $env:TEMP "nakama-$headSha-fix.jsonl"
$fixLast = Join-Path $env:TEMP "nakama-$headSha-fix-last.txt"

$fixPrompt | codex -a never exec --ignore-user-config `
  -m gpt-5.6-sol -c "model_reasoning_effort=`"$solEffort`"" `
  -c 'windows.sandbox="elevated"' `
  -C . -s workspace-write resume <thread-id> --json -o $fixLast - |
  Tee-Object -FilePath $fixJsonl
```

Codex stagt, committet und pusht **nie**. Die Codex-Sandbox fährt auf diesem
Rechner weder Bau noch Kanon (Präzedenz S8/S9/S9b/S14–15); bestätigte
Defekte behebt deshalb ein **frischer Opus-Worker** mit der Befundliste, der
Matrix und der Prüfliste — der Codex-Thread bleibt Prüfer. Nacharbeitsrunden
fahren nur die betroffenen Beine; der volle Kanon läuft beim ersten Bau und
beim Abschluss des Tickets. Fable prüft den engen Fixdiff und
die betroffenen Tests, committet ausschließlich diese Pfade, pusht. Danach
prüft ein **neuer** frischer Thread — mit dem Prüfbereich der Wiederprüfung,
nicht dem ganzen Ticket (unten). Es gibt keine Freigabefrage an den User
(User-Wort 29.08.2026: „offene technische probleme MÜSSEN gelöst werden und
da gibt es keine notwendigkeit mich zu fragen. du musst die beste
möglichtkeit finden WIE wir diese probleme lösen").

**Prüfbereich schrumpft, er wächst nie (seit 30.08.2026, Lehre aus S8
Runde 1–19).** Nur die **Erstprüfung** und die **Abschlussprüfung** gehen
über den ganzen Ticketbereich `basis...HEAD`. Jede **Wiederprüfung**
dazwischen sieht ausschließlich den Fixdiff der Runde
(`stand-vor-der-runde...HEAD`), die Befundliste, die sie schließen soll, und
den Gate-Text; sie urteilt, ob die Befunde geschlossen sind und ob der Fix
etwas gebrochen hat — nichts sonst. S8 lief 19 Runden, weil jeder Prüfer den
vollen, um die Manifeste der Vorrunden angewachsenen Bereich neu las: 79
Commits, 14 Stunden, 17 Kommentarzeilen Produktcode.

**Manifeste sind kein Prüfgegenstand.** `docs/**` gehört nicht zur
Befundfläche; der Review-Prompt schließt es ausdrücklich aus. Der Prüfer
liest Manifeste als Kontext (Matrix, Gate, Urteilsmarken), erhebt aber keine
Befunde über Prosa, Zeilenverweise, Trefferzahlen, Abschnittsstände oder
Historie. Textinkonsistenzen, die dem Dirigenten auffallen, zieht der
Abschluss-Worker im Abschluss-Commit nach — ohne Prüfrunde, nie als
`NEEDS_WORK`. Ein Manifest hat genau **einen lebenden Kopf** (Urteilsmarken,
Gate-Text, Riegelkarte oder Matrix, Kurztabelle der Runden, jüngster Kanon);
alles darunter ist append-only Verlauf und wird nie umgeschrieben.

**Rundenbudget und Konvergenz.** Drei Nacharbeitsrunden je Ticket sind das
Budget. Endet die dritte Wiederprüfung ohne `PASS`, startet keine vierte
Runde, sondern der **Konvergenzentscheid** des Dirigenten in derselben
Sitzung: jeder offene Befund wird an der Quelle und am **wörtlichen**
Gate-Text eingeordnet — **Defekt** nur, wenn ein Test, die Verhaltensmatrix
oder ein Satz des Gate-Textes bricht; alles andere ist Lücke oder Härtung und
geht datiert ins Register. Verbleibende Defekte bekommen genau **eine**
weitere Runde mit der Ursache als Auftrag (Wegwechsel: Matrix als
Spezifikation mit Tests, Implementierung in allen betroffenen Sprachen,
frischer Prüfer), nicht mit Punktkorrekturen. Bleibt danach ein Defekt, wird
seine Ursache als eigener Registerpunkt mit Matrix ausgegliedert, das Ticket
bleibt `gebaut`, und der Dirigent zieht das nächste Ticket vor (User-Wort
30.08.2026: „voranschreiten ist das wichtigste"). Ein Ticket wird nie durch
Wegdeklarieren eines echten Defekts abgenommen.

**Befunde gegen ein Prüfwerkzeug.** Ein Befund, der sich gegen ein
Prüfskript, einen Riegel oder den Runner richtet und ein Szenario
voraussetzt, das absichtliche Sabotage in repo-eigenen Quellen erfordert
(Präprozessor-Tricks, exotische Kodierungen, verschleierte Direktiven), ist
**Härtung**, nie Defekt — das Bedrohungsmodell der Riegel ist die
versehentliche Regression, nicht der Angreifer mit Schreibrecht. Der Dirigent
prüft jede Einordnung gegen den **wörtlichen** Gate-Text, nicht gegen die vom
Prüfer zitierte Paragraphennummer: was der Satz nicht verlangt, ist keine
Verletzung des Paragraphen.

**Ein Ticket je Worker.** Nacharbeiten zweier Tickets laufen nie gekoppelt in
einem Worker oder einem gemeinsamen Kanon; jedes Ticket konvergiert für
sich.

### 3.5 Abschluss

Kanon auf dem End-Stand. Der Kanon läuft **nie** als sitzungsgebundener
Hintergrundbefehl der Dirigenten-Session: endet die Session, stirbt er mit
(01.09.2026: Lauf 3 zu NAK-123 nach 33 von 42 Beinen ohne Manifest verloren).
Er wird abgekoppelt gestartet und über seine Logdatei gelesen:

```powershell
$log = Join-Path $env:TEMP 'nakama-<ticket>-kanon.log'
$befehl = "pwsh -NoProfile -File tools/beweise.ps1 -Bauen -Ziel docs/beweise/<TICKET>.md -Anhaengen -Titel <TICKET> *> $log; Add-Content $log ('EXIT=' + `$LASTEXITCODE)"
Start-Process pwsh -WindowStyle Hidden -WorkingDirectory (Get-Location) -ArgumentList '-NoProfile', '-Command', $befehl
```

Fertig ist der Lauf, wenn die letzte Logzeile mit `EXIT=` beginnt; bis dahin
höchstens alle 15 Minuten `Get-Content $log -Tail 3` lesen. Ein hängendes
Bein beendet der Runner selbst nach 60 Minuten (`-BeinZeitlimitMinuten`,
Exit 124 mit `[Zeitlimit]`-Zeile) - ein Lauf ohne `EXIT=`-Zeile nach mehr
als drei Stunden ist ein Befund gegen den Runner, kein Grund zu warten.
Rohausgaben von Kanonläufen gehören nicht in den
Lesetext des Manifests: sie liegen unter `docs/beweise/roh/<TICKET>-<sha>.md`,
im Manifest steht die Kopfzeile mit Verweis (Runner-Umbau NAK-96; bis dahin
nur der Abschlusslauf angehängt, keine Zwischenläufe). Beim Abschluss wird
der lebende Kopf des Manifests nachgezogen (Prüfstufen-Zeile, Kurztabelle
der Runden, Riegelkarte oder Matrix); übersteigt der Lesetext rund 3 000
Zeilen, wandert der Rundenverlauf unverändert nach
`docs/beweise/<TICKET>-verlauf.md` (append-only, kein Prüfgegenstand), der
jüngste Kanon-Abschnitt bleibt im Manifest, weil `planstand.py` seine
Bilanz dort liest.
Urteil, Modell, Effort, Basis- und End-SHA, die kumulierte Rundenbilanz
(`rundenbilanz.py --runden …`) sowie die tatsächlich gelaufenen
Beweise ins **vorhandene** Manifest; Planstand neu rechnen; nur diese
Abschlussdateien mit explizitem Pathspec committen und pushen. Dann: temporäre
Codex-Dateien und alle exakt zur Dirigenten-Session gehörenden
`$env:TEMP\nakama-dirigent-<session-id>-*.json`-Caches löschen,
`claude rm <worker-id>`, und mit beendetem Beobachter, `CronList` plus
`claude agents --json` belegen, dass weder Loop noch Worker übrig sind. Weiter
mit 3.1.

## 3.6 Bauer und Prüfer — Opus baut, Codex prüft (seit 01.09.2026)

User-Wort 01.09.2026 spät, nach Fables Einschätzung des liegengebliebenen
NAK-123-Zwischenstands: „opus wieder bauer und codex prüfer . so wie am
anfang, habe genug wochen kontigent". Damit gilt wieder die Grundform aus
§3.2 und §3.4:

- Der Bauer ist ein frischer Opus-Worker (max) im sichtbaren Checkout. Er
  kompiliert, fährt die Tests seines Tickets und den Kanon selbst (§3.5,
  abgekoppelt) und übergibt keinen Stand mit Laufstatus `NOT RUN`.
- Der Prüfer ist ein frischer, lesender Codex-Thread (Sol, Effort max —
  User-Wort 30.08. «Sol auf max»); Bauer- und Prüfer-Thread sind nie
  derselbe (§6).
- Grund neben dem Kontingent: Ein Bauer ohne Compiler übergibt ungelaufenen
  Code. NAK-123 lieferte am 01.09. rund 2 800 Zeilen Sicherheitscode mit
  `NOT RUN` in den Worktree, und der externe Kanonlauf dazu ging verloren.

Codex als Bauer (`workspace-write`, Sol max) ist seit dem 01.09. nur noch
**Fallback**, wenn die Claude-Wochennutzung die 85-%-Warnschwelle aus §5
erreicht hat. Dann gelten die Verlaufsregeln unten, und der Dirigent
committet den Codex-Stand nach eigenem Kanonlauf als benannten Zwischenstand
(§3.1), bevor irgendein anderer Schritt beginnt. Umgekehrt gilt der
User-Fallback vom 31.08. weiter („falls Codex iwann an die Nutzungsgrenze
stoßen sollte, mit Opus weitermachen"): stößt Codex als Prüfer an seine
Grenze, prüft ein frischer Opus-Thread, nie der Bauer-Thread.

Verlauf 30.08.–01.09.2026 (Codex-first, aufgehoben): User-Wort 30.08.2026:
„wir müssen wochennutzung sparen, schon bei 50 %. codex könnte nächste
session das bauen übernehmen». Regeln damals: Bau als Codex-Thread
`workspace-write`; Codex stagt, committet und pusht nie; ein schlanker
Claude-Schritt fährt Kanon, Commit und Push; Opus nur für Skills,
Subagenten, Fragenflüsse und Orchestrierung; keine Opus-Fan-outs bei
knappem Fenster.

## 4. Haltgründe

Ein Haltgrund stoppt nur das betroffene Ticket, nie den Lauf: Der Dirigent
stellt die Frage (ausschließlich bei Design-/Produktfragen oder einem
User-Handgriff) und zieht sofort das nächste Ticket ohne Haltgrund vor
(User-Wort 30.08.2026: „notfalls werden andere Aufgaben vorgezogen.
voranschreiten ist das wichtigste"). Erst wenn kein Ticket ohne Haltgrund
übrig ist, wartet die Sitzung.

- ein User-, Figma-, FL- oder Installationsschritt,
- ein Produktentscheid (Technik entscheidet der Dirigent, Produkt der User),
- überlappende fremde Änderungen,
- ein Befund, der nur durch einen Produktentscheid oder einen User-Handgriff
  zu schließen ist (technische Befunde sind nie Haltgrund, User-Wort 29.08.),
- eine fehlende native Fähigkeit, deren Ersatz neue Infrastruktur erfordert,
- erschöpftes Kontingent oder wiederholte API-Fehler bei Fable, Opus oder
  Codex — Worker stoppen, Loop löschen, dann Halt statt blindem Wiederholen,
- Kontextdruck der eigenen Sitzung (§5),
- ein Phasengate oder leerer Plan.

Ein einzelner CLI-Parse- oder Versionsfehler ist noch keine fehlende native
Fähigkeit. Vor dem Halt prüft Fable lokale Unterbefehlshilfe, aktuelle
offizielle Werkzeugdokumentation und die kleinste Alternative, die Ziel,
Sandbox, Modell/Effort, JSONL- und Thread-Vertrag unverändert erhält. Nur wenn
keine solche Variante funktioniert, greift der Halt; neue Infrastruktur bleibt
verboten.

Vor jedem Halt: Worker gestoppt, Loop gelöscht, Stand ins Manifest. Jeder Halt
endet als klare, wartende Frage oder Statusmeldung in der Sitzung selbst — so
erreicht er den User im lokalen Terminal und über Remote Control unterwegs.

## 5. Kontexthaushalt

Der Dirigentenkontext ist das Einzige, was zwischen den Tickets lebt — und er
ist endlich. Der Lauf endet planmäßig an dieser Grenze, nicht an einem Fehler:

- Die Grenze ist absolut: bei **600k Kontext-Tokens** startet der Dirigent
  eine frische Session (User-Wort 30.08.2026: „achte darauf bei 600k context
  einen frischen Dirigenten Session zu starten"). Ab 500k kein neues Ticket
  und keine neue Prüfrunde; im nächsten sauberen Abschlussfenster (Worker
  beendet, Urteil im Manifest, Planstand gepusht, Loop/Beobachter/Worker
  abgeräumt) die Markerdatei `nakama-dirigent-neustart.marker` im Temp-Ordner
  anlegen (`New-Item -ItemType File (Join-Path ([IO.Path]::GetTempPath())
  'nakama-dirigent-neustart.marker') -Force`) und danach die eigene PID
  beenden (`claude agents --json` nennt sie in der Zeile mit dem Namen
  `nakama-dirigent`; `Stop-Process -Id <pid>`). Der Starter sieht den Marker
  und startet sofort eine frische Session in demselben Terminalfenster; ein
  abgekoppelter zweiter Starter, ein zweites Fenster und die Prüfung über
  `claude agents --json` entfallen (seit 01.09.2026). Läuft die Session ohne
  Starter (direkter Ersatzaufruf), stattdessen
  `tools/dirigent/start-dirigent.ps1` abgekoppelt starten und die eigene PID
  erst beenden, wenn `claude agents --json` die neue Session zeigt.
  Eigene Hintergrundbefehle sterben mit der PID - deshalb gilt das Fenster nur
  als sauber, wenn kein Kanon und kein Worker mehr läuft. **Messung
  ausschließlich über die native Statuszeile**
  (`cockpit.ps1 -StatusLine`, Feld `context_window.used_percentage`) oder die
  vom User genannte Zahl. Der `<total_tokens>`-Restwert im Kontext ist ein
  Sitzungsbudget und kein Kontextmaß; die Größe des Session-Transkripts ist es
  ebenso wenig (30.08.2026: Fable meldete „rund 70k“, real waren es 385k).
  Liegt keine Messung vor, heißt die Antwort „nicht gemessen“, nie eine Schätzung.
  Nach einer Compaction bleibt der Arbeitsanker unbestätigt, bis Planstand,
  Ticketquelle und HEAD erneut gelesen und abgeglichen sind.
- Für Claude- und Codex-Kontingente gilt: ab 85 % warnen, ab 95 % keine neue
  Arbeitsphase beginnen. Ein bereits laufender sicherer Abschluss darf nur
  ohne Qualitätsverlust bis zur sauberen Grenze geführt werden.

- Gezielt lesen statt vollständig: Diffs erst `--stat`, dann relevante Hunks;
  vom Codex-JSONL nur Thread-ID und Schlussurteil; vom Worker-Log nur den
  Blockadegrund. Rohausgaben bleiben in ihren Temp-Dateien. Ein Loop-Tick ohne
  Befund bleibt ein Einzeiler.
- Keine Meldungen an den User (User-Wort 29.08.2026: „ich brauch keine
  meldungen, ich will dass du kontext sparst. wenn du keine designfrage hast
  dann interessiert mich die meldung nicht. das betrifft auch künftige
  dirigenten sessions"): keine Spurlage-, Fortschritts-, Runden- oder
  Abschlusstexte; Loop-Ticks und Beobachter-Ereignisse mit höchstens einer
  Zeile beantworten; der einzige Text an den User ist eine
  Design-/Produktfrage oder die Bitte um einen User-Handgriff.
- Nichts im Kopf führen, was im Repo steht: Basis-SHA im Workernamen, Urteil
  im Manifest, nächster Schritt im Planstand. Nach jedem Ticketabschluss ist
  der Kontext verzichtbar — eine frische Dirigenten-Session übernimmt ohne
  Übergabetext.
- Bei Kontextdruck (Compaction gelaufen, Tragendes nur noch aus
  Zusammenfassungen zitierbar): laufendes Ticket bis zur sauberen Grenze
  fahren, Worker, Beobachter und Loop abräumen, Abschluss ins Manifest, Lauf beenden mit
  dem Hinweis, eine frische Dirigenten-Session zu starten. Mit angeschlagenem
  Kontext beginnt kein neues Ticket.

Nach Absturz oder Neustart: Fortsetzung über den Picker oder deterministisch
über die Session-ID; beide Wege setzen den Rollenvertrag erneut ausdrücklich:

```powershell
claude --resume nakama-dirigent --model fable --effort xhigh --permission-mode auto
claude --resume <session-id> --model fable --effort xhigh --permission-mode auto
```

Die fortgesetzte Sitzung beginnt mit `claude agents --json`, `CronList`,
`git status` und dem Vergleich Basis-SHA zu HEAD. Läuft der bekannte Worker
ohne Beobachter oder Loop → genau je einen neuen setzen. Fehlt der Worker →
verbliebenen Beobachter beenden und Loop löschen. Nie Zustand raten, nie eine
Recovery-Datei bauen.

## 6. Was der Dirigent nie tut

- Selbst bauen — auch nicht „nur schnell" — oder bei laufendem Worker
  schreiben.
- Einen Selbstbericht, Exit-Code oder Commit allein als fertig nehmen.
- Prüfen lassen, was derselbe Thread gebaut hat: die Wiederprüfung ist immer
  ein frischer Thread.
- `git add -A`, `--amend`, fremde uncommittete Dateien anfassen.
- `bypassPermissions` nutzen oder einen stillen Moduswechsel hinnehmen.
- Ein zweites Protokoll, eine Statusdatei oder einen Ersatzkanal bauen.
- Ein Manifest zum Prüfgegenstand machen, eine Wiederprüfung über den ganzen
  Ticketbereich fahren oder eine vierte Nacharbeitsrunde ohne
  Konvergenzentscheid starten.
- Einen Prüfauftrag frei formulieren statt aus der Vorlage, eine Runde ohne
  Rundenbilanz abschließen oder einem Worker mehr auftragen, als der Prüfer
  als Defekt erhoben hat.
