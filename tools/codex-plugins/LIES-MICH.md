# Lokale Codex-Werkzeuge — Repo-Sicherung

Dieser Ordner ist die dauerhafte, Git-gesicherte Quelle für lokale
Codex-Werkzeuge, die zum Nakama-Projekt gehören. Er verhindert, dass eine
Editor- oder Plugin-Deinstallation die einzige Quelle löscht.

## Audio-Plugin-UX

- `audio-plugin-ux/` enthält die aktuelle Pluginquelle.
- `audio-plugin-ux-workspace/` enthält Benchmark, Baseline 0.2.1 und den
  reproduzierbaren Vergleichslauf.
- Codex-Installationscache und persönliche Marketplace-Konfiguration werden
  bewusst nicht versioniert.

Nach einer Neuinstallation wird die Pluginquelle aus diesem Ordner in ein
lokales Plugin-Quellverzeichnis kopiert und von dort mit dem Codex
Plugin-Creator installiert. Änderungen gehören zuerst in diese Repo-Kopie
oder müssen anschließend hierher zurückgespiegelt und getestet werden.
