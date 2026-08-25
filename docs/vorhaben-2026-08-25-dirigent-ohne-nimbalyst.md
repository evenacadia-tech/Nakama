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
| Fertig-Signal | `notifyOnComplete` | **entfällt** — `$p.WaitForExit()` |
| Netz gegen stillen Tod | `schedule_wakeup` (~1200 s) | **entfällt** — Timeout um den Prozess |
| Urteil einsammeln | freie Prosa | `--json-schema urteil.json` → validiertes JSON |
| Meldung an den User | Matrix `melden.py` | **identisch, hängt nicht an Nimbalyst** |

🔑 **Der Kern der Vereinfachung:** `notifyOnComplete` und `schedule_wakeup`
existieren nur, weil der Dirigent selbst ein LLM ist und nicht blockierend
warten kann. Ein Skript ruft `WaitForExit()`. Damit entfällt auch die im
Skill dokumentierte Falle (Z. 95–97): *„`lastActivity` ist KEIN
Aktivitätssignal — es stand 22 Minuten still, während die Session
committete."*

### Skizze

```powershell
foreach ($ticket in $offene) {
  $uuid = [guid]::NewGuid()
  $p = Start-Process pwsh -PassThru -ArgumentList '-NoExit','-Command',
       "claude -p '$prompt' --model opus --output-format json --session-id $uuid > $log"
  $p.WaitForExit()                      # sequenziell — Pflicht, s. u.
  $urteil = Get-Content $log | ConvertFrom-Json
  # bewerten, melden (melden.py), naechstes Ticket
}
```

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
| **Dirigent** | das PowerShell-Skript | deterministische Ablaufsteuerung, blockierendes Warten |
| **Bauer** | `claude -p` | frischer Kontext je Ticket |
| **Prüfer** | `codex review` | **anderes Modell** — stärker als nur eine frische Session |
| **Fixer** | `codex exec` | derselbe Prüfer behebt, was er gefunden hat |

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

### Ablauf je Ticket — alles in EINEM Terminal, sequenziell

```powershell
foreach ($ticket in $offene) {
  # 1. BAUEN (Claude, frischer Kontext)
  claude -p $bauPrompt --model opus --output-format json `
         --json-schema schemas/bau-urteil.json --session-id $uuid | Tee-Object $log

  $sha = git rev-parse HEAD

  # 2. PRUEFEN (Codex, anderes Modell)
  codex review --commit $sha --json | Tee-Object $reviewLog

  # 3. FIXEN, nur bei Befund
  if ($befund) {
    codex exec "Behebe: $befund. Beleg an der Quelle." `
               --output-schema schemas/fix-urteil.json --json
  }
  # 4. melden.py, dann naechstes Ticket
}
```

Kein `Start-Process`, kein zweites Fenster: Jeder Aufruf schreibt in
dasselbe Terminal und blockiert bis zum Ende. `Tee-Object` hält den Verlauf
zugleich auf der Platte fest.

### Was daraus noch zu klären ist

1. **Ist der Dirigent das Skript oder eine LLM-Session?** Diese Skizze macht
   das Skript zum Dirigenten — deterministisch, kein Wakeup nötig. Braucht
   der Dirigent echtes Urteilsvermögen (z. B. „ist S9 wirklich fertig?"),
   ruft das Skript dafür einen eigenen `claude -p`-Schritt mit Schema auf,
   statt selbst eine dauerhafte LLM-Session zu sein.
2. **Abbruchregel:** Wie oft darf Fixer→Prüfer kreisen, bevor das Ticket an
   den User geht? Vorschlag: zweimal, dann Matrix-Meldung und Stopp.
3. **Wer setzt die Urteilsmarke** in `docs/beweise/`? Nach heutiger Regel nur
   ein Prüfer — das spräche für Codex, verlangt aber, dass er das Manifest
   schreiben darf.

### Aufwand mit Codex-Rollen

Die Schätzung oben (~4 h) bleibt gültig; die Prüfer-/Fixer-Stufe kostet
zusätzlich ~45 min, weil `codex review` das Review-Format bereits mitbringt
und nicht selbst gebaut werden muss.

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
