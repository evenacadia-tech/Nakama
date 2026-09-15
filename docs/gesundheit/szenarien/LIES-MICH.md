# Szenarien des Laufzeit-Arms

Jede Datei hier ist ein Szenario für `tools/fl/szenario.py` (Plan S25e,
Register NAK-286, KONZEPT §4.6): eine Folge von Aktionen des FL-Controller-
Skripts mit Erwartungen je Antwortfeld. `tools/fl/laufzeit.ps1` fährt alle
`*.json` dieses Ordners nach grünem Kanon gegen den installierten Bau im
Diagnoseprojekt (`eq-copilot/fixtures/fl/Nakama-Diagnose.flp`).

| Datei | Frage | Stand |
|---|---|---|
| `bereitschaft.json` | Antwortet FL, ist das richtige Projekt mit beiden Plugins offen, fließt Ton durch Sondenspur und Master? | erste Fassung 12.09.2026 |
| `fenster.json` | Lassen sich das FL-Hauptfenster und das Fenster von Nakama Probeeq ohne Vordergrundwechsel als Bild erfassen (ein einfarbiges Bild ist verfehlt)? | Etappe 4 NAK-286, 15.09.2026 (M-21) |
| `nulltest-host.json` | Ist der Render des Diagnoseprojekts im Auslieferungszustand (`FL64.exe /R`, Runner) bitidentisch zur Quelle? VERSATZ, KETTE, Formatfehler oder kein Render enden mit Szenario-Exit 5; Render mit Verarbeitung und ohne Slots brauchen Karte U43 | Etappe 4 NAK-286, 15.09.2026 (M-60 bis M-65, M-75) |
| `snapshot-runde01.json` | Stimmen Snapshot und Rahmen beider Rollen über den Briefkasten innerhalb eines Umlaufs mit dem Referenzausschnitt der Quelle überein (Bänder nach F-28, Runde-01-Anker nur roh)? Frischer Start vorher | Etappe 4 NAK-286, 15.09.2026 (M-66 bis M-68, M-81) |
| `u40-aktivitaetsgate.json` | Welche Messwerte liefert das Aktivitätsgate an den leisen Stellen S1, S2, S3 des Testtracks (Rohdaten, keine Deutung, Karte U40)? | Etappe 4 NAK-286, 15.09.2026 (M-69 bis M-72) |

Noch anzulegen (S25e): Nulltest im echten Host (Render über `FL64.exe /R`
mit und ohne Verarbeitung), Snapshot über den Diagnose-Briefkasten, Karte
U40 (Aktivitätsgate nach NAK-283 Etappe 5).

Format und Erwartungsarten stehen im Kopf von `tools/fl/szenario.py`.
Aktionen sind die des Controller-Skripts
(`C:\Users\phili\Projekte\fl-studio-mcp\fl_controller\device_FLStudioMCP.py`,
Funktion `dispatch_command`). Pegel: `peak_max` 0 = Stille, 1 = 0 dBFS.
