# Vorhaben 25.08.2026 — Dirigent ohne Nimbalyst + Werkzeug-Entrümpelung

**Für eine frische Session gedacht. Alles hier ist in der Session vom 25.08.
gemessen, nicht geschätzt — die Belege stehen dabei, damit nichts neu
getestet werden muss.**

Anlass, User-Wort 25.08.: *„mir ist das nämlich hier etwas ‚zuviel'
drumherum"* — bezogen auf die Nimbalyst-Abhängigkeit des Dirigenten.
Status: **Vorhaben, nicht beauftragt.** Nichts davon ist gebaut.

---

## Teil 1 — Dirigent im Terminal statt in Nimbalyst

### Was der Dirigent heute braucht

`.claude/skills/dirigent/SKILL.md` (228 Zeilen) hängt an genau drei
Nimbalyst-Werkzeugen:

| Skill-Stelle | Werkzeug |
|---|---|
| §3.1 Baum frei? | `mcp__nimbalyst-host__list_recent_sessions` |
| §3.2 Bau-Session | `mcp__nimbalyst-host__spawn_session` |
| §3.3 Netz | `mcp__nimbalyst-host__schedule_wakeup` |

Dazu §3.6: Die Matrix-Antwort landet in der Session aus `config.json` →
`nimbalyst.session_id`.

### Was bereits BEWIESEN ist (25.08., nicht wiederholen)

| Behauptung | Beleg aus dem Testlauf |
|---|---|
| Verschachtelte Claude-Session aus laufender Session heraus geht | `claude -p "…" --output-format json` → **Exit 0, 11,2 s**, `session_id` im JSON |
| Sichtbares Terminal lässt sich öffnen | `Start-Process pwsh -ArgumentList '-NoExit','-Command',$cmd -PassThru` → Fenster war für den User sichtbar |
| Keine Admin-Rechte nötig | Kind lief in derselben Windows-`SessionId 3` wie der Explorer |
| CLI kann alles Nötige | `--bg`, `--session-id <uuid>`, `--output-format json`, `--json-schema`, `--resume`, `--fork-session`, `--model`, `claude agents --json --cwd <p> --all` („for scripting; does not require a TTY") |

⚠️ **Zwei eigene Messfehler, damit sie niemand wiederholt:**

1. `MainWindowHandle` ist **kein** Sichtbarkeitsbeweis für Konsolenfenster —
   das Fenster gehört dem Konsolen-Host, nicht `pwsh`. Gegenprobe: Nimbalyst
   selbst hat ebenfalls Handle `0` und ist sichtbar. Das Fenster war da,
   die Messung war falsch.
2. Ein Regex `[a-zA-Z_]+` für Tool-Namen verschluckt Namen mit Bindestrich
   (`context7__query-docs`). Immer `[a-zA-Z0-9_-]+`.

### Abbildung Nimbalyst → Terminal

| Dirigent braucht | Nimbalyst | Terminal |
|---|---|---|
| Frische Session | `spawn_session` | `claude -p --model opus --session-id <uuid> --output-format json` |
| Läuft schon was? | `list_recent_sessions` | `claude agents --json --cwd . --all` + `git status --porcelain` |
| Fortschritt | `get_session_status` | **Exit-Code + neue Commits** |
| Fertig-Signal | `notifyOnComplete` | **entfällt** — der Werkzeugaufruf ist synchron |
| Netz gegen stillen Tod | `schedule_wakeup` (~1200 s) | **entfällt** — Timeout des Werkzeugaufrufs |
| Urteil einsammeln | freie Prosa | `--json-schema urteil.json` → validiertes JSON |
| Meldung an den User | Matrix `melden.py` | **identisch, hängt nicht an Nimbalyst** |

🔑 **Der Kern der Vereinfachung:** `notifyOnComplete` und `schedule_wakeup`
existieren, weil Nimbalysts `spawn_session` **asynchron** ist — es setzt ab
und kehrt sofort zurück, also muss der Dirigent später geweckt werden. Ein
`claude -p` über das Shell-Werkzeug ist **synchron**: Der Dirigent bleibt im
Werkzeugaufruf stehen und bekommt das Ergebnis zurück. Damit entfällt auch
die im Skill dokumentierte Falle (Z. 95–97): *„`lastActivity` ist KEIN
Aktivitätssignal — es stand 22 Minuten still, während die Session
committete."* Ein Exit-Code lügt nicht.

**Sequenziell ist Pflicht, nicht Geschmack:** Skill §3.1 —
`eq-copilot/build/` ist ein geteiltes Verzeichnis, zwei parallele Läufe
kollidieren. Deshalb ist `--bg` unnötig; blockierendes `claude -p` ist der
direktere Weg. `--bg` lohnt nur bei echter Parallelität.

### Festlegungen (Technik, meine Entscheidung)

- **PowerShell, nicht Git Bash.** Grund steht in `CLAUDE.md`
  „Maschinen-Landminen": Bash-Heredocs verändern Backslashes in
  Windows-Pfaden, Pipes verdecken Exitcodes. Bei einem Orchestrator, der
  Exit-Codes auswertet und Pfade weiterreicht, ist das die falsche
  Fehlerklasse. `tools/beweise.ps1` ist ohnehin PowerShell.
- **Fortschritt nie an einem Statusfeld messen**, immer an Exit-Code und
  neuen Commits.
- `--model` explizit setzen — ohne das liefert die CLI ihr Default
  (im Test kam `claude-fable-5`, nicht Opus). Entspricht `inheritModel: true`.
- `--session-id` vorher vergeben: dann weiß das Skript, wo das Transkript
  landet (`~/.claude/projects/<projekt>/<uuid>.jsonl`) und kann ohne Board
  hineinsehen.

### Aufwand (AI-Zeit, mit Iteration)

| Teil | grob |
|---|---|
| Kernschleife | ~45–60 min |
| Robustheit: Timeout, Abbruch, Baum-frei-Prüfung, Logging | ~60 min |
| Urteilsschema + Anbindung `docs/plan/plan.json` und Urteilsmarken | ~45 min |
| Matrix-Meldung | ~10 min (existiert) |
| Dirigent-Skill §3.1–3.3 umschreiben | ~30 min |
| Echte Durchläufe + Nachjustieren | ~60 min |

**Summe rund 4 Stunden.** Empfohlener erster Schritt: **kein Skript**,
sondern ein Wegwerf-Durchlauf von Hand mit `claude -p` über EIN Ticket —
entscheidet in ~20 min, ob der Rest sinnvoll ist.

### Was verloren geht

1. Das Kanban-Board. Ersatz: die laufende Ausgabe im Terminal selbst.
2. Reinsehen in eine laufende Session: teilweise über `claude --resume <id>`.
3. Der Lauf überlebt das geschlossene Terminal nicht — außer über Task
   Scheduler oder `Start-Process -WindowStyle Hidden`.

---

## Teil 1b — ANFORDERUNG des Users (25.08., wörtlich)

> „unser wrapper muss aber codex mit einschließen. 1 dirigenten session ->
> spawnt 1 bauer session, codex ist immer prüfer und fixer. alle laufen in
> einem terminal im powershell. das ist meine anforderung"

### Rollen

| Rolle | Werkzeug | Warum |
|---|---|---|
| **Dirigent** | **echte Claude-Session, Modell Fable**, interaktiv im Terminal | urteilt, entscheidet, meldet — **kein Skript** |
| **Bauer** | `claude -p`, vom Dirigenten aufgerufen | frischer Kontext je Ticket |
| **Prüfer** | `codex review` | **anderes Modell** — stärker als nur eine frische Session |
| **Fixer** | `codex exec` | derselbe Prüfer behebt, was er gefunden hat |

> **User-Wort 25.08.:** *„nein dirigent ist eine richtige claude session mit
> fable. kein script oder sonst was. beim dirigenten spart man nicht."*

🔑 **Korrektur einer früheren Fehlanalyse (stand vorher in diesem Blatt):**
Es hieß hier, `notifyOnComplete` und `schedule_wakeup` entfielen, weil „ein
Skript blockierend warten kann, ein LLM nicht". Der zweite Halbsatz ist
falsch. Ein LLM-Dirigent, der `claude -p` oder `codex exec` über sein
Shell-Werkzeug aufruft, **wartet blockierend** — der Werkzeugaufruf ist
synchron und liefert das Ergebnis zurück. Am 25.08. selbst vorgeführt:
`claude -p` aus einer laufenden Claude-Session, 11,2 s blockiert, Exit 0,
Ergebnis im Tool-Output.

Die Wakeup-Mechanik kompensiert also **Nimbalysts asynchrones
`spawn_session`** (fire and forget), nicht eine Eigenschaft von LLMs. Ein
blockierender CLI-Aufruf braucht sie nicht. Der Dirigent darf deshalb ein
vollwertiges Modell sein und trotzdem ohne Weckmechanik auskommen.

⚠️ **Die eine echte Grenze: das Werkzeug-Zeitlimit.** Shell-Werkzeugaufrufe
laufen typisch in ein Timeout (bei Claude Code max. 10 min). Eine Bau-Session
dauert länger. Lösung ist **nicht** ein Wakeup, sondern der
Hintergrundmodus des Shell-Werkzeugs: Der Aufruf wird abgesetzt, der Harness
verfolgt den Prozess und meldet dem Dirigenten die Fertigstellung. Für
kurze Schritte (Prüfen, Fixen) reicht blockierend.

🔑 **Warum Codex als Prüfer die stärkere Lösung ist:** Der heutige Dirigent
begründet den Prüfmechanismus mit frischem Kontext (SKILL.md Z. 24–27). Ein
anderes Modell schlägt das — es hat andere blinde Flecken, nicht nur einen
leeren Kontext. Im Repo bereits belegt: `NAK-78` hält fest, dass
`/c-review` (25 TRUE_POSITIVE) und `/rust-review` (17 TRUE_POSITIVE, **0
FALSE_POSITIVE**) über Codex echte Befunde lieferten.

⚠️ Gegenprobe aus derselben Historie: Fremdmodell-Befunde sind nicht
automatisch wahr — beim Gemini-Vorfall waren 8 von 9 Befunden echt, einer
frei erfunden (`tools/hooks/fremdmodell-riegel.sh`). Der Fixer darf deshalb
nur beheben, was er an der Quelle belegen kann; das Urteil gehört ins
Manifest, nicht in eine Sitzungsnotiz.

### Verifizierte Codex-Bausteine (25.08. an `codex --help` gemessen)

| Befehl | Zweck |
|---|---|
| `codex exec` (alias `e`) | „Run Codex non-interactively" |
| `codex review` | „Run a code review non-interactively" — genau die Prüferrolle |
| `codex review --commit <SHA>` / `--base <BRANCH>` / `--uncommitted` | Prüfumfang wählen |
| `codex exec --output-schema <FILE>` | JSON-Schema für die **finale Antwort** — das Urteil kommt strukturiert zurück |
| `codex exec --json` | Events als JSONL auf stdout |
| `codex exec -C <DIR>` / `--add-dir` | Arbeitsverzeichnis |
| `codex exec -m <MODEL>` / `-s <SANDBOX>` | Modell- und Sandbox-Wahl |
| `codex apply` | letzten Agent-Diff als `git apply` übernehmen |

Symmetrie zu Claude: `--output-schema` (Codex) entspricht `--json-schema`
(Claude). Beide Seiten liefern damit validiertes JSON statt Prosa — das ist
die Bedingung dafür, dass ein Skript entscheiden kann.

`~/.codex/config.toml` steht bereits auf `approval_policy = "never"` und
`sandbox_mode = "danger-full-access"`, läuft also ohne Rückfragen durch.

### Ablauf je Ticket — alles in EINEM Terminal

Der Dirigent wird **einmal** gestartet und läuft durch:

```powershell
claude --model fable          # ein Terminal, eine Session, der Dirigent
```

Von dort ruft er je Ticket über sein Shell-Werkzeug auf — jeder Aufruf
blockiert, jede Ausgabe landet im selben Terminal:

```powershell
# 1. BAUEN (Claude, frischer Kontext, laenger -> Hintergrundmodus)
claude -p $bauPrompt --model opus --output-format json `
       --json-schema schemas/bau-urteil.json --session-id $uuid | Tee-Object $log

$sha = git rev-parse HEAD

# 2. PRUEFEN (Codex, anderes Modell) - kurz, blockierend
codex review --commit $sha --json | Tee-Object $reviewLog

# 3. FIXEN, nur bei Befund
codex exec "Behebe: $befund. Beleg an der Quelle." `
           --output-schema schemas/fix-urteil.json --json

# 4. melden.py
```

Zwischen den Schritten **urteilt der Dirigent** — er liest Exit-Code, JSON,
Diff und Manifest und entscheidet, ob gefixt, wiederholt, gemeldet oder das
nächste Ticket begonnen wird. Genau dafür ist er ein Modell und kein
`foreach`.

Kein `Start-Process`, kein zweites Fenster. `Tee-Object` hält den Verlauf
zugleich auf der Platte fest.

### Was daraus noch zu klären ist

1. ~~Ist der Dirigent das Skript oder eine LLM-Session?~~ **Beantwortet
   25.08.:** echte Claude-Session mit Fable, kein Skript. *„beim dirigenten
   spart man nicht."*
2. **Langläufer.** Baut ein Ticket länger als das Werkzeug-Timeout
   (bei Claude Code max. 10 min), muss der Bau-Aufruf in den
   Hintergrundmodus des Shell-Werkzeugs — der Harness meldet dem Dirigenten
   die Fertigstellung. Zu messen: wie lange ein echtes Ticket wirklich baut.
3. **Abbruchregel:** Wie oft darf Fixer→Prüfer kreisen, bevor das Ticket an
   den User geht? Vorschlag: zweimal, dann Matrix-Meldung und Stopp.
4. **Wer setzt die Urteilsmarke** in `docs/beweise/`? Nach heutiger Regel nur
   ein Prüfer — das spräche für Codex, verlangt aber, dass er das Manifest
   schreiben darf.
5. **Modellwahl je Rolle:** Dirigent Fable (gesetzt), Bauer Opus
   (`--model` explizit, sonst nimmt die CLI ihr Default — im Test kam
   `claude-fable-5`), Prüfer/Fixer Codex nach `config.toml`.

### Aufwand mit Codex-Rollen

Die Schätzung oben (~4 h) bleibt gültig; die Prüfer-/Fixer-Stufe kostet
zusätzlich ~45 min, weil `codex review` das Review-Format bereits mitbringt
und nicht selbst gebaut werden muss.

---

## Teil 1c — Der Skill wird ÜBERNOMMEN, nicht neu geschrieben

> **User-Wort 25.08.:** *„aber der dirigenten skill wird exakt so übernommen
> wie er aktuell ist nur technisch angepasst, weil das hat super
> funktioniert"*

`.claude/skills/dirigent/SKILL.md` hat 289 Zeilen. Betroffen sind davon
**rund 35** — alles andere bleibt wörtlich stehen.

### Bleibt unangetastet (die Substanz)

- **§1** Warum es die Rolle gibt — Frischkontext als Prüfmechanismus.
- **§2** Woher der nächste Schritt kommt (PLAN-STAND → NEXT-SESSION →
  Bauaufteilung, mit der Warnung, dass die Bauaufteilung handgepflegt und
  am 23.08. selbst veraltet war).
- **§3.4** Messen und Urteilen — Belegpflicht mit (a) worauf du dich stützt
  und (b) was du NICHT geprüft hast; die vier Indizien; *„Glaube keinem
  Selbstbericht"*; die Warnung, dass ein roter Riegel nicht automatisch ein
  Befund ist.
- **§3.6** Meldepflichten — Matrix als einziger Kanal, die drei
  Pflichtauslöser, die Drei-Zeilen-Form, „kein Rückfallkanal mehr", die
  Zweiter-Client-Lehre.
- **§4** Wo du hältst (alle sechs Punkte).
- **§5** Protokollformat.
- **§6** Was du nie tust.

### Technisch zu ersetzen

| Stelle | heute | künftig |
|---|---|---|
| §3.1, Z. 55 | `mcp__nimbalyst-host__list_recent_sessions` | `claude agents --json --cwd . --all`; für die in Z. 59 verlangte Codex-Prüfung zusätzlich `codex exec resume --last` bzw. ein Blick in `~/.codex/sessions/` |
| §3.2, Z. 65 | `spawn_session` mit `inheritModel: true`, `notifyOnComplete: true` | `claude -p --model <opus> --output-format json --json-schema … --session-id <uuid>`; `inheritModel` wird zu **explizitem** `--model` (die CLI nimmt sonst ihr Default — im Test `claude-fable-5`) |
| §3.2, Z. 66 | „**Nicht** `useWorktree`" | entfällt technisch (CLI arbeitet ohnehin im cwd), der **Grund** bleibt als Satz stehen |
| §3.3, Z. 84–86 | `notifyOnComplete` + `schedule_wakeup` als Netz | **entfällt** — der Werkzeugaufruf blockiert. Neu stattdessen: Langläufer über den Hintergrundmodus des Shell-Werkzeugs |
| §3.3, Z. 95–97 | `lastActivity` taugt nicht, nur `updatedAt` + Commits | wird zu: **Exit-Code + neue Commits**. Die Lehre bleibt, ihr Träger ändert sich |
| §3.5, Z. 138 | Prüf-Session = frische Claude-Session | **`codex review --commit <SHA>`** — anderes Modell statt nur anderer Kontext. Der Basispunkt-Hinweis (Z. 141–144) bleibt wörtlich: Basis selbst ausrechnen, nie aus einem Dokument übernehmen |
| §3.5, Z. 152 | Nacharbeit-Session spawnen | `codex exec --output-schema …` — der Fixer ist derselbe, der gefunden hat; die Regel *„urteilen darf nur, wer nicht gebaut hat"* bleibt damit gewahrt |

### §3.3 „Kein Dauerpoll" — Regel bleibt, Anlass entfällt

Der Satz gegen Polling (Z. 88–93) wird technisch gegenstandslos: Ein
blockierender Aufruf hat nichts zu pollen. Er bleibt trotzdem stehen, weil
seine Begründung den Kanalwechsel überlebt — und weil das Datei-Postfach
unten sonst zum Poll-Einfallstor würde.

### ⚠️ Die einzige echte Lücke: der Matrix-RÜCKkanal

Ausgehend ist unkritisch: `melden.py` übergibt dem Dienst nur Aufträge und
kennt Nimbalyst nicht.

**Eingehend hängt fest.** `C:\Users\phili\.claude\matrix-bridge\config.json`
trägt einen Block `nimbalyst` mit `workspace_path` und `session_id`; dazu
gibt es ein eigenes Modul `nimbalyst.py`. Die Antwort des Users wird darüber
als Prompt in eine Nimbalyst-Session eingespeist. Ohne Nimbalyst gibt es
diesen Weg nicht mehr — und §3.6 Auslöser 3 (blockende Entscheidung) hängt
genau daran.

**Vorschlag (nicht entschieden): Datei-Postfach.** Der Dienst legt eingehende
Nachrichten als Datei ab; der Dirigent liest sie **in §3.1**, wo er ohnehin
`git status` prüft. Das ist kein Dauerpoll, sondern ein Schritt im ohnehin
stattfindenden Zyklus — die Regel aus §3.3 bleibt gewahrt. Zu klären: was
passiert, wenn der Dirigent gerade in einem langen Bau-Aufruf steht und eine
blockende Antwort eintrifft.

Aufwand für die Skill-Anpassung: ~30 min. Der Rückkanal ist ein eigenes
Stück, ~45 min, und betrifft `dienst.py` außerhalb des Repos.

---

## Teil 2 — Werkzeug-Entrümpelung (offen, nicht entschieden)

**Warum das hierher gehört:** Der Testlauf meldete **56 713 Tokens
Cache-Creation** für einen Prompt mit zwei Eingabe-Tokens. Das ist der
Systemprompt inklusive **aller MCP-Tool-Schemas** — und den zahlt jede
frisch gespawnte Dirigenten-Session neu. Aufräumen wirkt hier
multiplikativ, nicht additiv.

Messgrundlage: **1050 echte MCP-Aufrufe** über 400+ Sessions aller Projekte
(`~/.claude/projects`), Muster `"name":"mcp__…__…","input"`.

### Block A — nie benutzt, ~112 Tool-Schemas

`nimbalyst-excalidraw` (19) · `nimbalyst-trackers` (18) ·
`nimbalyst-situational` (17) · `codebase-memory-mcp` (14) ·
`claude.ai Google Drive` (11, **1** Aufruf jemals) ·
`claude.ai Google Calendar` (9) · `nimbalyst-sqlite-browser` (9) ·
`nimbalyst-mindmap` (7) · `context7` (2) · `nimbalyst-csv-spreadsheet` (2) ·
`nimbalyst-datamodellm` (2) · `nimbalyst-developer` (1) ·
`sequential-thinking` (1) · Nimbalyst-Memory-Extension (nie indexiert) ·
claude.ai-Connectors Canva/Cloudflare/Indeed/Supabase/Vercel (nicht einmal
authentifiziert).

Schärfster Einzelbefund: `codebase-memory-mcp` **plus** der Skill
`codebase-memory-exploring`, der ausdrücklich verlangt *„Do not use Grep,
Glob — use this first"* — in 400+ Sessions **kein einziger** Aufruf.

### Behalten (belegt genutzt)

`plugin_playwright` 220 · `fl-studio` 156 (99 FL-Studio, **57 Nakama**) ·
`claude_ai_Figma` 140 · `nimbalyst`+`-host`+`-browser` 276 · `blender` 22.

**`nimbalyst-host` ist NICHT auf der Streichliste** — daran hängt der
Dirigent, solange Teil 1 nicht gebaut ist.

### Zu entscheiden

- Doppelung `plugin_playwright` (24 Tools) ↔ `nimbalyst-browser` (16) —
  beide werden benutzt, zusammen 40 Schemas für eine Aufgabe.
- Skills sind **kein** lohnendes Ziel: ein Skill kostet eine Zeile
  Beschreibung, kein Tool-Schema. `dsp-cookbook`,
  `plugin-architecture-patterns`, `daw-compatibility-guide` sind Fachreserve
  für dieses Projekt.

### Wer was abschalten kann

| Ebene | Wer |
|---|---|
| Nimbalyst-Extensions | Claude — `mcp__nimbalyst-host__extension_set_enabled` |
| `codebase-memory-mcp`, `context7`, `sequential-thinking` | Claude — `claude mcp remove <name> -s user` |
| claude.ai-Connectors | **nur der User**, über die claude.ai-Connector-Einstellungen |

---

## Verwandte Vorgeschichte

- `design/abnahmen/2026-08-25-openwiki-entfernt.md` — OpenWiki restlos
  entfernt; dort steht die Messmethode **„Erwähnung ist kein Zugriff"**, die
  auch Teil 2 trägt.
- `docs/review-2026-08-25-audio-plugin-ux.md` — Review des Codex-Skills.
