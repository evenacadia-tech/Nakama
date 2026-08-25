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
`spawn_session`** (fire and forget), nicht eine Eigenschaft von LLMs. Der
Dirigent darf deshalb ein vollwertiges Modell sein und braucht trotzdem
keinen eigenen Weckdienst: Wo blockierend gearbeitet wird, kommt das
Ergebnis direkt zurück; wo im Hintergrund gearbeitet wird, meldet der
Harness die Fertigstellung.

⚠️ **Der Bau läuft trotzdem im Hintergrund — und zwar nicht wegen des
Zeitlimits.** Der tragende Grund ist die Meldepflicht aus §3.6 Auslöser 2
(siehe Teil 1b unten): Ein Dirigent, der in einem blockierenden Bau-Aufruf
steht, kann nicht stündlich melden. Das Werkzeug-Zeitlimit (bei Claude Code
max. 10 min) kommt als zweiter, schwächerer Grund dazu. Für die kurzen
Schritte — Prüfen und Fixen — bleibt blockierend richtig.

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

### Was daraus noch zu klären ist — alles beantwortet

1. ~~Ist der Dirigent das Skript oder eine LLM-Session?~~ **Beantwortet
   25.08.:** echte Claude-Session mit Fable, kein Skript. *„beim dirigenten
   spart man nicht."*
2. ~~Langläufer und Werkzeug-Timeout.~~ **Beantwortet 25.08.:** *„eine
   session baut solange wie sie braucht. dafür ist fable da das im auge zu
   haben. das hat bis jetzt immer geklappt."* Kein Zeitzaun um den Bau. Der
   Bau-Aufruf läuft im **Hintergrundmodus** des Shell-Werkzeugs, damit der
   Dirigent handlungsfähig bleibt; der Harness meldet die Fertigstellung.
3. ~~Abbruchregel der Fix-Schleife.~~ **Steht bereits im Skill**, §4 Punkt 5:
   *„Zweimal in Folge gescheitert am selben Ticket. Nicht ein drittes Mal
   dasselbe versuchen — das ist die Schleife, vor der `CLAUDE.md` warnt."*
   Da der Skill wörtlich übernommen wird, gilt sie unverändert. Die Frage
   war überflüssig.
4. ~~Wer setzt die Urteilsmarke in `docs/beweise/`?~~ **Beantwortet 25.08.:**
   *„codex hat genausoviel rechte wie claude."* Codex prüft, urteilt und
   schreibt sein Urteil ins Manifest. Die Skill-Regel *„urteilen darf nur,
   wer nicht gebaut hat"* bleibt gewahrt — gebaut hat Claude.
5. **Modellwahl je Rolle:** Dirigent Fable (gesetzt), Bauer Opus
   (`--model` explizit, sonst nimmt die CLI ihr Default — im Test kam
   `claude-fable-5`), Prüfer/Fixer Codex nach `config.toml`.

### 🔑 Warum der Bau NICHT blockierend laufen darf

Das folgt aus dem Skill selbst und war in der ersten Fassung dieses Blatts
falsch: §3.6 Auslöser 2 verlangt **mindestens stündliche** Meldung, auch
während ein Ticket noch läuft — *„S9 läuft seit 40 min, Kanon 12/28.
Schweigen ist für den User nicht von einem Absturz zu unterscheiden."*
Prüfbar gemacht durch `py -3.13 melden.py --letzte` (Exit 1 = überfällig).

Ein Dirigent, der im blockierenden Bau-Aufruf steht, **kann diese Pflicht
nicht erfüllen**. Deshalb:

| Schritt | Modus |
|---|---|
| Bauen (lang, offenes Ende) | **Hintergrund** — Dirigent bleibt handlungsfähig, meldet stündlich, sieht bei Bedarf in `git log` und Manifest |
| Prüfen (`codex review`) | blockierend |
| Fixen (`codex exec`) | blockierend |

Das ist kein Dauerpoll (§3.3 bleibt gewahrt): Der Dirigent wird bei
Fertigstellung geweckt und sieht dazwischen nur dann nach, wenn die
Meldepflicht ihn ohnehin zum Hinsehen zwingt.

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
blockierender Aufruf hat nichts zu pollen. Er bleibt trotzdem wörtlich
stehen, weil seine Begründung den Kanalwechsel überlebt — ein Poll erzeugt
je Runde Nachrichten, ohne je früher fertig zu sein als das Ergebnis, auf
das er wartet. Wer später einen Wartemechanismus nachrüstet, soll das
gelesen haben.

### Der Matrix-RÜCKkanal entfällt ersatzlos

**User-Entscheid 25.08.:** *„ohne nimbalyst kann ich doch ganz normal die
claude remote verbindung über die claude handy app benutzen. daher ist das
egal"* — auf die Frage, wie eine Antwort in die Dirigenten-Session
zurückfindet.

Der Dirigent wird deshalb mit **Remote Control** gestartet:

```powershell
claude --model fable --remote-control nakama-dirigent
```

Damit erreicht der User die **laufende lokale Session** direkt aus der
Claude-App und antwortet dort hinein. Der Umweg über Matrix ist für die
Rückrichtung nicht mehr nötig.

**Folgen:**

- `matrix-bridge/config.json` → Block `nimbalyst` (`workspace_path`,
  `session_id`) und das Modul `nimbalyst.py` werden **gegenstandslos**.
- Im Skill fällt §3.6 Z. 195–202 (Antwort landet in der
  `nimbalyst.session_id`, Suchreihenfolge laufende Claude → ruhende Claude →
  laufende Codex → ruhende Codex) **ersatzlos weg** statt ersetzt zu werden.
- **Ausgehend bleibt Matrix unverändert bestehen**: `melden.py` kennt
  Nimbalyst nicht und erfüllt weiter die drei Pflichtauslöser aus §3.6. Der
  Grund dafür trägt weiter — eine Push-Nachricht erreicht den User, ohne
  dass er die App offen hat.
- Der zuvor hier veranschlagte Aufwand für einen Ersatz-Rückkanal (~45 min)
  **entfällt**.

Aufwand für die Skill-Anpassung damit: **~30 min**, und es bleibt beim
Gesamtrahmen von rund 4–5 Stunden.

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
