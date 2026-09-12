# Szenarien des Laufzeit-Arms

Jede Datei hier ist ein Szenario für `tools/fl/szenario.py` (Plan S25e,
Register NAK-286, KONZEPT §4.6): eine Folge von Aktionen des FL-Controller-
Skripts mit Erwartungen je Antwortfeld. `tools/fl/laufzeit.ps1` fährt alle
`*.json` dieses Ordners nach grünem Kanon gegen den installierten Bau im
Diagnoseprojekt (`eq-copilot/fixtures/fl/Nakama-Diagnose.flp`).

| Datei | Frage | Stand |
|---|---|---|
| `bereitschaft.json` | Antwortet FL, ist das richtige Projekt mit beiden Plugins offen, fließt Ton durch Sondenspur und Master? | erste Fassung 12.09.2026 |

Noch anzulegen (S25e): Nulltest im echten Host (Render über `FL64.exe /R`
mit und ohne Verarbeitung), Snapshot über den Diagnose-Briefkasten, Karte
U40 (Aktivitätsgate nach NAK-283 Etappe 5).

Format und Erwartungsarten stehen im Kopf von `tools/fl/szenario.py`.
Aktionen sind die des Controller-Skripts
(`C:\Users\phili\Projekte\fl-studio-mcp\fl_controller\device_FLStudioMCP.py`,
Funktion `dispatch_command`). Pegel: `peak_max` 0 = Stille, 1 = 0 dBFS.
