# Nakama — Session-Workspace

Dieses Folder ist der **Session-Anker** für Claude-Arbeit am Nakama-Projekt
(transparentes Berater-VST3 für FL Studio, Legacy-Name EQ-Copilot).

Hier liegen Kontext, Wissen und Handoffs — **nicht der Code**. Der Code lebt in
`C:\Users\phili\FL-Studio\eq-copilot\` (+ Broker in `plugin-hub-app`); dieses
Workspace hat per `.claude/settings.json` Zugriff darauf.

| Datei | Zweck |
|---|---|
| `CLAUDE.md` | Tragende Invarianten, Pfade, Bau- und Beweiskommandos |
| `docs/plugin-wissen.md` | Wie das VST3-Plugin heute funktioniert |
| `docs/design-stand.md` | An welchem Design-Prototypen wir gerade arbeiten |
| `docs/offene-punkte.md` | Durables Offen-Set (nie stillschweigend löschen) |
| `docs/handoffs/` | Automatische Session-Handoffs (Cap 5) |
| `tools/hooks/` | SessionStart-Primer, Depth-Primer, Handoff-Hooks, Guards |

Sessions in diesem Folder starten mit injizierter Systemkarte + Live-Git-Stand
des FL-Studio-Repos. Code-Commits gehen ins FL-Studio-Repo, Wissens-Commits
hierher.
