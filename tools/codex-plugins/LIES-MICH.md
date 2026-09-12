# Lokale Codex-Werkzeuge — Repo-Sicherung

Dieser Ordner ist die dauerhafte, Git-gesicherte Quelle für lokale
Codex-Werkzeuge, die zum Nakama-Projekt gehören. Er verhindert, dass eine
Editor- oder Plugin-Deinstallation die einzige Quelle löscht.

## Audio-Plugin-UX

- `audio-plugin-ux/` enthält die kanonische, im Nakama-Repository bearbeitete
  Pluginquelle.
- Der frühere Benchmark-Workspace (Baseline 0.2.1, Iterationen 1–4) wurde am
  12.09.2026 entfernt; sein letzter Stand liegt im Verlauf bis Commit
  `f9750046` unter `tools/codex-plugins/audio-plugin-ux-workspace/`.
- Codex-Installationscache und persönliche Marketplace-Konfiguration werden
  bewusst nicht versioniert.

Nach einer Neuinstallation wird die Pluginquelle aus diesem Ordner in ein
lokales Plugin-Quellverzeichnis kopiert und von dort mit dem Codex
Plugin-Creator installiert. Das externe Plugin-Repository dient nur als
Veröffentlichungsspiegel. Änderungen entstehen und bestehen zuerst in dieser
Repo-Kopie; beide Orte werden nicht unabhängig voneinander bearbeitet.
