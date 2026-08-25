# 2026-08-25 — OpenWiki restlos entfernt

## Der Entscheid (bindend, User-Wort 25.08.2026)

> „entferne das komplette openwiki konstrukt"

## Was vorausging: eine Messung, keine Meinung

Der User hatte gefragt, wie oft OpenWiki in den letzten Tagen von Claude oder
Codex genutzt wurde, **um Kontext zu erhalten**. Gemessen wurden die
Transkripte beider Werkzeuge (`~/.claude/projects/C--Users-phili-Projekte-Nakama/`,
53 Dateien / 226 MB, und `~/.codex/sessions/2026/08/`):

| Werkzeug | Befund |
|---|---|
| **Claude** | **Kein einziger Kontextzugriff.** Genau eine Session (`3fd8dfef`, 23.08.) berührte OpenWiki überhaupt — 10 MCP-Aufrufe (`begin`/`inspect`/`resolve`/`finish`), also ein **Pflegelauf**. Vier weitere Sessions nur verwaltend (`git diff -- openwiki/`, `Test-Path`). Die übrigen ~48 Sessions: nur der MCP-Systemprompt-Text. |
| **Codex** | **Genau ein** echter Kontextzugriff: 25.08., 10:29 lokal — `Get-Content -Raw 'openwiki/plugin/editor-and-diagnostics.md'` vor dem Schreiben eines eigenen UX-Reviews. Alles davor (22./23./24.08.) war Aufbau und Pflege des Wikis. |
| **Beide** | Codex hat `openwiki/` **viermal aktiv aus Suchen ausgeschlossen** (`--glob '!openwiki/**'`, 24.08. 3×, 25.08. 1×) — also als Rauschen behandelt, nicht als Quelle. |

Methodischer Hinweis für spätere Leser: Die Rohzahlen („openwiki" kommt 716×
bei Claude und 1352× bei Codex vor) sind wertlos. Der MCP-Server schreibt
seinen Instruktionstext in **jeden** Systemprompt — nachgewiesen über das
Muster „OpenWiki exposes deterministic lifecycle", exakt 1× in 36 Sessions.
Erst die Aufrufmuster messen Verhalten.

## Der aufgehobene Gegen-Entscheid

Am 22.08. hatte der User gesagt:

> „das workspace hier baut auf openwiki als kontextsystem auf"

Daraus war in `docs/NEXT-SESSION.md` die stehende Anweisung geworden: *„nicht
gegen `docs/` abwaegen, nicht als Zusatzquelle behandeln, **nicht abschalten
vorschlagen**"*. Diese Anweisung ist mit dem 25.08. **aufgehoben** — das
neuere und speziellere User-Wort schlägt das ältere. Die Stelle in
`NEXT-SESSION.md` ist entsprechend durchgestrichen und datiert, nicht
gelöscht: der alte Wortlaut bleibt nachlesbar.

## Was entfernt wurde (vollständig)

**Im Repo** (Commit dieses Änderungssatzes):

- `openwiki/` — 36 Dateien (13 Wiki-Seiten, 13 Claims-Sidecars, Indizes,
  `.last-update.json`)
- `AGENTS.md` — bestand ausschließlich aus dem OpenWiki-Block
- `CLAUDE.md` — Arbeitsregel „OpenWiki ist ein optionaler Evidenzindex…" und
  der `<!-- OPENWIKI:START/END -->`-Abschnitt
- `docs/NEXT-SESSION.md` — Klarstellung vom 22.08. als aufgehoben markiert
- `docs/offene-punkte.md` — NAK-45 geschlossen (war der unabgeschlossene
  OpenWiki-Lauf)

**Außerhalb des Repos:**

| Was | Ort |
|---|---|
| MCP-Registrierung Claude | `~/.claude.json` — via `claude mcp remove openwiki -s user` |
| MCP-Registrierung Codex | `~/.codex/config.toml` — Block `[mcp_servers.openwiki]` |
| Verwaister Marker | `~/.codex/config.toml` — `# OPENWIKI:MCP:END` stand ohne BEGIN mitten in `[shell_environment_policy.set]` |
| Trust-Eintrag | `~/.codex/config.toml` — `[projects.'…\nimbalyst-openwiki-test']` |
| Skill (Claude) | `~/.claude/skills/openwiki/` — 8 Dateien |
| Skill (Codex/Agents) | `~/.agents/skills/openwiki/` — 8 Dateien |
| Testprojekt | `~/Workspace/nimbalyst-openwiki-test/` — 85 Dateien |
| Server | `~/Workspace/.openwiki-agent-bridge/` — 40 220 Dateien, 412,7 MB |
| Laufende Prozesse | 5 `node.exe` (3× `--host codex`, 2× `--host claude`) beendet |

Ein GitHub-Actions-Workflow existierte nicht mehr (am 23.08. gelöscht, siehe
`NEXT-SESSION.md`); es gab nichts, was das Wiki neu erzeugt hätte.

## Der Rückweg (falls er je gebraucht wird)

Die Gegenrichtung gehört dokumentiert, auch wenn sie nicht beschritten wird:

- **Server:** öffentlicher Klon, `git clone https://github.com/langchain-ai/openwiki.git`.
  Der gelöschte Stand war `337f890` (22.08.2026), **null** lokale Änderungen —
  es ging kein eigener Code verloren.
- **Wiki-Inhalt:** liegt vollständig in der Git-Historie dieses Repos; der
  letzte Stand ist der Commit vor diesem.
- **Registrierung:** `claude mcp add` bzw. ein `[mcp_servers.openwiki]`-Block.

## Geltung

Nichts davon suchen, nichts davon wiederbeleben, nicht als Kontextquelle
vorschlagen. Maßgeblich bleiben Source, Tests, Beweise und die in `CLAUDE.md`
verlinkten Fachdateien.

**Nicht angetastet**, weil historische Belege: `docs/beweise/G1.md`,
`docs/beweise/SONDE-008.md`, `docs/beweise/SONDE-009.md`,
`docs/dirigent/protokoll.md`, `docs/archiv/`. Sie erwähnen OpenWiki als Teil
ihres Datums und dürfen das weiter tun.
