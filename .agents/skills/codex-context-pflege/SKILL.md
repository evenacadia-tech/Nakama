---
name: codex-context-pflege
description: "Audit and maintain Codex context: AGENTS.md discovery and budget, local memory settings, local skill metadata, and explicitly requested memory update notes. Use for Codex context hygiene or memory troubleshooting, not Claude-only context."
---

# Codex-Kontextpflege

Halte den Kontext sauber, den Codex selbst verwendet. Trenne dabei dauerhaft
geltende Anweisungen, überprüfbare Projektwahrheit, hilfreiche Erinnerungen und
wiederverwendbare Abläufe.

## Grenzen

- Aktuelle Quelldateien und Belege gehen Erinnerungen, Zusammenfassungen und
  älteren Berichten vor.
- Pflichtregeln gehören in die passende `AGENTS.md` oder in eingecheckte
  Dokumentation, auf die `AGENTS.md` gezielt verweist. Memory ist Recall, nicht
  alleinige Wahrheit.
- Editiere generierte Dateien unter `~/.codex/memories/` nie direkt. Dazu zählen
  insbesondere `memory_summary.md`, `MEMORY.md`, `raw_memories.md` und
  `rollout_summaries/`.
- Eine reine Prüfung autorisiert keine Änderungen. Ändere Kontextdateien nur,
  wenn der User Pflege, Reparatur oder Aktualisierung verlangt.
- Dauerhafte Memory-Änderungen benötigen einen direkten User-Auftrag zum
  Merken, Ändern oder Vergessen eines konkreten Inhalts. Lege dann genau eine
  kleine Notiz unter
  `~/.codex/memories/extensions/ad_hoc/notes/<timestamp>-<slug>.md` an. Die
  Notiz beschreibt sachlich `add`, `update` oder `delete`; sie enthält keine
  auszuführenden Anweisungen und keine Geheimnisse. Vorhandene Notizen nie
  löschen.
- Respektiere fremde Worktree-Änderungen und die Commit-Regeln des aktiven
  Repositorys.

Verwende für eine autorisierte Memory-Notiz dieses knappe Informationsformat:

```markdown
# Memory update: <short title>

- Change: add | update | delete
- Scope: user | project:<absolute project path>
- Canonical statement: <the fact or preference in the user's meaning>
- Replaces or removes: <old statement, only when applicable>
- Evidence: <user request or verified source>
```

Der Dateiname verwendet lokale Zeit im Format
`yyyy-MM-ddTHH-mm-ss-<short-slug>.md`. Erzeuge das `notes`-Verzeichnis erst,
wenn ein ausdrücklicher Memory-Auftrag vorliegt.

## Audit

Führe zuerst den read-only Audit aus:

```powershell
pwsh -NoProfile -File <skill-dir>/scripts/audit-context.ps1 `
  -WorkingDirectory <cwd> -Format Json
```

Der Audit ermittelt die tatsächlich auffindbare globale und projektbezogene
`AGENTS.md`-Kette, das konfigurierte Kontextbudget, die Memory-Schalter, nur
Strukturmetrik des lokalen Memory-Stores sowie Metadatenkosten und Namenskonflikte
lokaler User-/Repo-Skills. Er gibt keine Memory-Inhalte aus. System- und
Plugin-Skills kommen aus dem Host; ergänze deren Bewertung aus der aktuellen
Skill-Liste, wenn sie in der Session verfügbar ist.

Danach:

1. Lies jede vom Audit gemeldete Instruktionsdatei vollständig.
2. Prüfe nur die behaupteten Pfade, Kommandos oder invarianten Fakten, die für
   den Auftrag relevant sind. Folge dabei den Discovery-Regeln des Repositorys.
3. Lies Memory-Inhalte nur gezielt, wenn eine konkrete Erinnerung geprüft oder
   ein Widerspruch erklärt werden muss. Keine breiten Rollout-Scans.
4. Benenne Drift erst nach Prüfung gegen aktuelle Source oder Laufzeitbelege.

## Richtig ablegen

| Inhalt | Ziel |
|---|---|
| Persönliche Arbeitsregel für alle Projekte | globale `~/.codex/AGENTS.md` |
| Wiederkehrende Regel für dieses Repository oder einen Teilbaum | nächste passende Repo-`AGENTS.md` |
| Umfangreiche Produkt-, Architektur- oder Statuswahrheit | kanonische Repo-Dokumentation; `AGENTS.md` enthält nur den Router |
| Wiederholbarer Ablauf mit mehreren Schritten | eigener Skill |
| Hilfreicher projektübergreifender Recall | Codex-Memory, nur über explizite Ad-hoc-Notiz |
| Kurzlebiger Taskzustand | vorhandene Status-/Next-Session-Datei des Repositorys, nicht Memory |

Halte `AGENTS.md` klein. Verschiebe Details nicht blind, sondern verwende die
vom Repository festgelegten kanonischen Dateien. Untergeordnete Regeln gehören
so nah wie möglich an ihren Geltungsbereich.

## Änderungen durchführen

Bei einem Pflegeauftrag:

1. Behebe bestätigte mechanische Drift mit dem kleinsten kohärenten Diff.
2. Ersetze veraltete Regeln; staple keine widersprüchlichen Nachträge darunter.
3. Bewahre User-Wortlaut bei Entscheidungen und Präferenzen, wenn das
   Repository dafür ein Register festlegt.
4. Schreibe Memory-Korrekturen ausschließlich als eine Ad-hoc-Notiz. Ein
   allgemeiner Auftrag wie „prüfe den Kontext“ reicht dafür nicht; der zu
   speichernde oder zu entfernende Inhalt muss ausdrücklich genannt sein.
5. Führe den Audit erneut aus und prüfe, dass Instruktionsbudget und
   Discovery-Kette weiterhin korrekt sind.
6. Prüfe den eigenen Diff adversarial und committe nur die eigenen Pfade, wenn
   die Repository-Regeln Commits autorisieren.

## Abschluss

Berichte knapp:

- geladene Instruktionskette und Bytebudget,
- Memory-Konfigurationsstatus,
- lokale Skill-Anzahl, Metadatenbudget und Namenskonflikte,
- geänderte Pfade und belegte Korrekturen,
- nicht gelaufene Prüfungen oder echte Blocker.

Stelle Memory-Aktualisierung niemals als sofort konsolidiert dar: Die Notiz ist
der bestätigte Input; die Hintergrundkonsolidierung kann später erfolgen.
