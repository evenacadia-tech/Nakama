# Context-Hygiene-Playbook (Nakama)

Hält `MEMORY.md` und `CLAUDE.md` frisch, schlank und korrekt, damit jede
Session mit richtigem Kontext startet — keine veralteten Behauptungen, kein
aufgeblähter Index, keine stillen Lücken. `/freshen` führt dieses Playbook
end-to-end aus. Struktur abgeleitet vom Sonifold-Playbook (20.08.2026);
alle Anker hier sind NAKAMA-eigen — nie mit Sonifold/Strukturkomplex/
evenacadia/FL-Studio mischen.

Das ist **Kontext**-Hygiene, nicht Code-Qualität: der Beweis-Kanon
(GoldenTest, NullTest, MarkierungTest, pluginval, PaintBench, cargo test)
gehört NICHT hierher.

## Budget — MEMORY.md ist ein Cache fester Größe, kein Log

Der Harness lädt nur die ersten **~24,4 KB (24 986 Bytes)** von `MEMORY.md`;
darüber wird still abgeschnitten — die unteren Einträge erreichen Claude
nicht mehr. Der SessionStart-Hook meldet die Überschreitung
(`MEMORY.md is X KB (limit: 24.4KB)`) — diese Warnung ist ein harter
Trigger: sofort `/freshen`, vor allem anderen.

- **Arbeitsziel ≤ 22 KB** (22 528 Bytes) — ~2 KB Luft für die laufende Session.
- **Netto-Null-Wachstum:** Ein Lauf endet nie größer als er begann, außer
  eine neue tragende Landmine kam dazu UND wurde durch Eviction
  gleichwertigen alten Inhalts bezahlt.
- Messen am Anfang (Phase A) und am Ende (Phase F), jede Runde:
  ```bash
  wc -c ~/.claude/projects/C--Users-phili-Projekte-Nakama/memory/MEMORY.md
  ```

**Die weitere Always-on-Fläche** (jede Session vor dem ersten Tool-Call
geladen): globales `~/.claude/CLAUDE.md` (~5 KB, User-Besitz — nie
auto-editieren) + Repo-`CLAUDE.md` (~15 KB seit 21.08. — von Claude gepflegt, Drift
wird hier GEFIXT, Wachstum wird gemeldet) + `MEMORY.md` (Hebel). Gesamt
~20 KB (Stand 20.08.2026) — jede Runde neu messen und im Report nennen.

**Der Korpus** (die Memory-*Dateien* auf Platte) ist die dritte Fläche —
Recall-Qualität, nicht Kontextkosten. Stand 20.08.2026: ~10 Dateien
(junges Repo; die Sonifold-Marke „80–90 gesund" gilt hier NICHT als Ziel).
Wächst nur die Klasse der datierten `project_session_*`-Memos — deren
**Cap ist 6** (CLAUDE.md „Arbeitsweise": eine Lehre pro Datei, mit dem
Warum; Falsches löschen statt stapeln).

## System-of-Record-Karte

Vor jedem „redundant"-Urteil prüfen, ob die kanonische Quelle die
Information wirklich trägt (`ls`/`grep`, nicht Karte glauben). Ja →
Kandidat für Eviction. Nein → tragend, behalten.

| Memory-Klasse | Kanonische Quelle |
|---|---|
| Design-Sessions (Abnahmen, Figma-Stände) | `design/abnahmen/` + `design/assets/figma/` (seit 21.08.; Prisma-Studie geparkt unter `eq-copilot/design/prisma-studie/`) |
| Produktwahrheit, User-Entscheide (mit Zitat) | `CLAUDE.md` (Wahrheitskern + Register) |
| Plugin-/Broker-Sessions (Architektur, Datenfluss, IPC) | `docs/plugin-wissen.md` + `eq-copilot/docs/M*-BEFUND.md` + git log |
| Offene Punkte / NAK-IDs | `docs/offene-punkte.md` (durabel; schließen NUR mit ID + Commit-SHA, nie still löschen) |
| Nächste Schritte | `docs/NEXT-SESSION.md` |
| Session-Übergaben | `docs/hub/hub.json` + die Briefing-Seite (seit 22.08.); `docs/NEXT-SESSION.md` für den einen nächsten Schritt. Die Auto-Handoffs unter `docs/handoffs/` sind am 22.08. entfallen — sie wurden geschrieben, aber nie gelesen (Commit `ef1efee`) |
| Verworfene Wege (nie neu vorschlagen) | CLAUDE.md „Was NICHT mehr gilt" + `eq-copilot/design/archive/LIES-MICH.md` + `docs/archiv/LIES-MICH.md` |

## Phasen

### Phase A — Inventar

1. `MEMORY.md` **mit dem Read-Tool von Platte** lesen (liefert auch den
   Teil hinter der Trunkierungsgrenze).
2. CLAUDE.md-Bestand **gitignore-bewusst**: `git ls-files '**/CLAUDE.md'
   CLAUDE.md` (Stand 20.08.2026: nur das Root-CLAUDE.md) — nie
   `find`/`Glob` (Worktree-Duplikate).
3. Memory-Verzeichnis listen:
   `ls ~/.claude/projects/C--Users-phili-Projekte-Nakama/memory/`.
4. Fläche messen (`wc -c` s. o.), MEMORY.md-Bytes notieren (Phase F misst
   dagegen), Korpus zensieren (Zahl je Klasse; `project_session_*` gegen
   Cap 6).

### Phase B — Drift-Erkennung

Jede Zahl-/Listen-/Pfad-Behauptung mit frischem Kommando prüfen, nie aus
Erinnerung. **Meta-Regel:** Ein Prüfkommando auf einen verschobenen Pfad
liefert still 0 und lässt Veraltetes „verifiziert" aussehen — erst
Pfad-Existenz, dann Zählung; gerottete Kommandos in DIESEM Playbook im
selben Lauf reparieren.

| Claim-Typ | Prüfung |
|---|---|
| Pfad-Tabelle + alle `docs/…`-Verweise im CLAUDE.md | je Pfad `[ -e ]`-Schleife |
| Build-Targets (EqCop*) | `grep -oE 'EqCop[A-Za-z_]+' eq-copilot/plugin/CMakeLists.txt \| sort -u` |
| Broker-Binärnamen | `broker/Cargo.toml` (`name =`) + `ls broker/src/bin/` |
| Pipe-Namen (v1 / m2probe) | `grep -rl 'evenacadia\.eq-copilot' broker/src eq-copilot/plugin/src` |
| NAK-IDs in MEMORY.md/NEXT-SESSION | gegen `docs/offene-punkte.md` (offen vs. geschlossen) |
| „Stand DD.MM."-Zeilen im CLAUDE.md | gegen `docs/NEXT-SESSION.md`; der Wahrheitskern trägt sein Datum selbst |
| Remote-/Repo-Claims | `git remote -v` |
| Schema-Versionen (eq-ipc v2, eq-snapshot v3 …) | `eq-copilot/schemas/` |

Cross-Doc-Regel: Ein in einem Doc gefixter Wert ist in den anderen noch
alt — den WERT repo-weit greppen (`git grep`), jede lebende Kopie
abgleichen (NEXT-SESSION.md, plugin-wissen.md, design/abnahmen,
offene-punkte.md). Historische/datierte Zeilen (Handoffs, Befund-Docs mit
Datum) sind als Zeitpunkt-Aussagen korrekt — nur als AKTUELL präsentierte
Altwerte sind Drift.

### Phase C — Bloat-Erkennung

Signal 0 = Master-Gate: MEMORY.md > 22 KB ⇒ Kompression PFLICHT, Lauf
endet netto kleiner. Dann:
1. **Index-Zeilen > 250 Zeichen** (mechanisch): `awk 'length>250'` — jede
   komprimieren; Detail lebt in der verlinkten Datei, der Hook trägt nur
   den tragenden Fakt.
2. **`project_session_*` über Cap 6** oder überholt: je Memo den
   kanonischen Artefakt-`ls`-Test (Karte oben) — existiert er, einmalige
   Fakten falten, Datei löschen, Index-Zeile im selben Rewrite entfernen.
3. **Stale Topic-Memos** (`project_*`, `reference_*`): Body an den
   heutigen Stand anpassen, wenn die Landmine hält, aber Details
   gedriftet sind (z. B. „nächster Schritt" längst erledigt).
4. **Volatile SHA-/Status-Claims**: „Stand @ SHA" / „nicht gemerged"
   rottet mit dem nächsten Commit — durabel formulieren (was zu TUN ist).

### Phase D — Lücken-Erkennung

`git log --oneline -20` gegen CLAUDE.md-/NEXT-SESSION.md-Abdeckung. Für
jeden Kandidaten erst `ls`/`grep`, ob das Genannte im Code existiert
(Commit-Messages können Phantome nennen). Mechanische Lücke (Zahl, Pfad,
Zeile) → fixen. Architektur-Prosa oder Design-Urteil → dem User
vorlegen (Surface ≠ Menü).

### Phase E — Ausführen

1. CLAUDE.md-Drift-Fixes (Phase B), dann mechanische Lücken (Phase D).
2. Memory-Datei-Edits (Bodies aktualisieren, Stale löschen nach ls-Test).
3. `MEMORY.md` als EIN Write am Ende, **nebenläufigkeitssicher**: erst
   `cp MEMORY.md MEMORY.md.bak` (rollierend), unmittelbar vor dem Write
   neu lesen, parallele Edits einfalten statt überschreiben (Datei ist
   untracked — kein Git-Undo).
4. Strukturelle Lücken vorlegen: EIN konkreter Vorschlag mit file:line +
   ein Satz + welcher kreative Input fehlt. Nie Optionsmenü, nie
   „soll ich / your call".

### Phase F — Selbstaudit

- Budget: ≤ 22 KB Ziel, Hard-Fail > 24,4 KB, nicht größer als Phase-A-Wert.
- `awk 'length>250'` leer (bzw. bewusst benannte Ausnahmen).
- Jede Memory-Datei von `MEMORY.md` erreichbar (`(datei.md)`-Link ODER
  `[[slug]]`) — 0 echte Waisen.
- Alle in diesem Lauf editierten Zahlen stichprobenartig gegen frische
  Kommandos gegenprüfen.
- `docs/offene-punkte.md`: kein Eintrag still verschwunden.

### Phase G — Qualitätspass

1. **Frontmatter**: jede Datei hat `name:`, `description:`,
   `metadata.type` ∈ {user, feedback, project, reference}; description
   spiegelt den BODY-Stand (Body-Update ohne description-Update ist der
   häufigste stille Drift).
2. **Why/How**: `feedback_*`/`project_*` tragen Begründung; bei
   `feedback_*` darf NUR eine fehlende Why/How-Zeile ergänzt werden — nie
   umformulieren oder abschwächen (User-Besitz).
3. **Redundanz**: gleicher Fakt in zwei Dateien → in die kanonische
   falten. `feedback_*` ist NIE ein CLAUDE.md-Duplikat (Datei = Warum +
   Historie, Regel = Digest).
4. **Hook-Informativität**: Index-Zeile, die nur den Titel paraphrasiert,
   → auf den tragenden Fakt umschreiben.
5. **Selbstbeschreibung**: `/freshen`-Launcher-Phasen ↔ dieses Playbook
   abgleichen, wenn sich Phasen ändern.

## Harte Regeln

- **MEMORY.md ≤ 22 KB Ziel / 24,4 KB Ceiling, Netto-Null-Wachstum** —
  darüber = stille Trunkierung.
- **Nur MEMORY.md wird frei umgeschrieben.** Globales `~/.claude/CLAUDE.md`
  nie anfassen; Repo-CLAUDE.md: mechanischen Drift fixen, strukturelles
  Wachstum melden.
- **`feedback_*.md` nie auto-editieren** (Ausnahme: fehlende Why/How-Zeile
  ergänzen). Das Entscheide-Register in CLAUDE.md (wörtliche User-Urteile) und
  `.claude/kreativ-freigabe.md` (Schleusen-Marker) sind ebenso tabu.
- **Nie eine Memory-Datei löschen ohne ls-bestätigtes kanonisches
  Artefakt.** Index komprimieren, nicht die Dateien.
- **Ein backup-first Write für MEMORY.md** — `.bak`, Re-Read, einfalten.
- **Gitignore-bewusste Werkzeuge** (`git ls-files`, `git grep`, Grep-Tool)
  für Inventar und Sweeps.
- **`docs/offene-punkte.md` nie still kürzen** — schließen nur mit ID +
  Commit-SHA (CLAUDE.md-Regel).
- **Surface ≠ Menü** — ein konkreter Vorschlag, User lenkt mit einem Wort.
- **Nichts erfinden** — Drift-Fixes sind mechanisch; neue
  Architektur-/Design-Prosa braucht User-Input (Kreativ-Schleuse!).

## Wann laufen lassen

- Sofort bei der SessionStart-Warnung `MEMORY.md is X KB (limit: 24.4KB)`.
- Wöchentlich; nach großen Schüben (≥3 Commits in einem Subsystem); nach
  Löschen/Umbenennen von Modulen, Blättern oder Docs; wenn eine
  CLAUDE.md-Behauptung beim ersten Lesen falsch wirkt; wenn
  `project_session_*` das Cap 6 reißt.

## Report am Ende (drei Zeilen)

Budget + Korpus (KB vs. 22 KB; Dateizahl; CLAUDE.md-KB bei Wachstum) ·
Was geändert (Zahlen, nicht Prosa) · Was vorgelegt (file:line + ein Satz +
benötigter Input). Ein sauberer Lauf ohne Änderungen ist ein gültiges
Ergebnis.
