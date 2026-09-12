# Nakama

Plugin-Familie für FL Studio, die beim Mischen berät: **Nakama Gen** (zwei
Oberflächen: Quellen, Befunde und Advisor; zentrale EQ-Bedienung mit
vollwertigem Master-EQ) und **Nakama Probeeq** (misst passiv auf dem Bus,
beliefert Gen und führt mit zugeschaltetem EQ Gens Fernsteuerung auf ihrem
Bus aus); Bundle **Nakama Studio**. Technik: JUCE 8 / C++20 / CMake +
eigenständiger Rust-Broker (Named Pipes). Code, Bundle und Pipes tragen bis
NAK-30 den Legacy-Namen EQ-Copilot (`EqCop*`).

Dieses Repo hält Code, Verträge, Beweise, Design und technisches Wissen. Die
visuelle Phase läuft in Figma (`fable-dummy`); Layout-Wahrheit ist die
technische Skizze unter `design/skizze/`, verbindlich sind nur datierte
Abnahmen in `design/abnahmen/`.

| Wohin | Wofür |
|---|---|
| `CLAUDE.md` | Wahrheitskern, Arbeitsregeln, Invarianten, Bauen und beweisen |
| `docs/PLAN-STAND.md` | Gerechneter Planstand und der nächste Schritt (`py -3.13 tools/plan/planstand.py`) |
| `docs/plugin-wissen.md` | Wie das Plugin heute funktioniert |
| `docs/FL-Nakama-Sonden-Design-Entwurf.md` | Technischer Entwurf der Sondenfamilie (Fassung 0.5) |
| `design/docs/funktions-und-bedien-blueprint.md` | Funktions- und Bedienarchitektur beider Apps |
| `docs/bauaufteilung-sonden.md` | Sessions, Gates, Prüfstufen |
| `docs/beweise/` | Beweismanifeste je Ticket, Rohausgaben unter `roh/` |
| `docs/offene-punkte.md` | Register der offenen Punkte und Entscheide (nie still kürzen) |
| `wissen/INDEX.md` | Einstieg für eingeordnetes externes Wissen |
| `docs/archiv/` · `eq-copilot/design/archive/` · `eq-copilot/design/prisma-studie/` | Verlauf, nie Vorgabe |
| `tools/beweise.ps1` | Ein Befehl für den ganzen Beweis-Kanon |

Bauen und Beweisen: `CLAUDE.md`, Abschnitt „Bauen und beweisen".
