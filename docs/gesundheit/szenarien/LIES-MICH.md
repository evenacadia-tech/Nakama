# Szenarien des Laufzeit-Arms

Jede Datei hier ist ein Szenario für `tools/fl/szenario.py` (Plan S25e,
Register NAK-286, KONZEPT §4.6): eine Folge von Aktionen des FL-Controller-
Skripts mit Erwartungen je Antwortfeld. `tools/fl/laufzeit.ps1` fährt alle
`*.json` dieses Ordners nach grünem Kanon gegen den installierten Bau im
Diagnoseprojekt (`eq-copilot/fixtures/fl/Nakama-Diagnose.flp`).

| Datei | Frage | Stand |
|---|---|---|
| `bereitschaft.json` | Antwortet FL, ist das richtige Projekt mit beiden Plugins offen, fließt Ton durch Sondenspur und Master? | erste Fassung 12.09.2026 |
| `fenster.json` | Lassen sich das FL-Hauptfenster und das Fenster von Nakama Probeeq ohne Vordergrundwechsel als Bild erfassen (ein einfarbiges Bild ist verfehlt)? Ein Plugin-Fensterbild unter 200 × 100 Pixel ist ein eingeklappter FL-Wrapper und endet mit Szenario-Exit 5, Grund `eingeklappt` (P-17; Aufklappen im Diagnoseprojekt ist Handgriff K-286-2, Karte U43) | Etappe 4 NAK-286, 15.09.2026 (M-21); P-17 Etappe 5, 15.09.2026 |
| `nulltest-host.json` | Ist der Render des Diagnoseprojekts im Auslieferungszustand (`FL64.exe /R`, Runner) bitidentisch zur Quelle? VERSATZ, KETTE, Formatfehler oder kein Render enden mit Szenario-Exit 5. Der Render mit Verarbeitung (Schritt `verarbeitung_ein`, Projekt `Nakama-Diagnose-Verarbeitung.flp`) und ohne Slots (Schritt `ohne_slots`, Projekt `Nakama-Diagnose-Referenz.flp`) braucht diese Projekte aus Karte U43 neben dem Diagnoseprojekt (Weg R2, M-64): liegen sie, kopiert der Runner sie, vergleicht ihre SHA-256 am Anfang und am Ende (P-18) und rendert sie vor dem FL-Start in eigene Ordner; `verarbeitung_ein` muss von der Quelle abweichen, `ohne_slots` dieselben SHA-256 des Datenbereichs tragen wie der Render im Auslieferungszustand (P-21); fehlt ein Projekt, endet sein Schritt mit Szenario-Exit 5 | Etappe 4 NAK-286, 15.09.2026 (M-60 bis M-65, M-75); P-18 Etappe 5, 15.09.2026; P-21 Nacharbeit 1 der Abschlussprüfung, 15.09.2026 |
| `snapshot-runde01.json` | Stimmen Snapshot und Rahmen beider Rollen über den Briefkasten innerhalb eines Umlaufs mit dem Referenzausschnitt der Quelle überein (Bänder nach F-28, Runde-01-Anker nur roh)? Frischer Start vorher | Etappe 4 NAK-286, 15.09.2026 (M-66 bis M-68, M-81) |
| `u40-aktivitaetsgate.json` | Welche Messwerte liefert das Aktivitätsgate an den leisen Stellen S1, S2, S3 des Testtracks (Rohdaten, keine Deutung, Karte U40)? | Etappe 4 NAK-286, 15.09.2026 (M-69 bis M-72) |

Gebaut in S25e (NAK-286, Etappen 2 bis 5): Nulltest im echten Host
(`nulltest-host.json`, Render im Auslieferungszustand gegen die Quelle; der
Render mit Verarbeitung und ohne Nakama-Slots ist gebaut, P-21, und misst mit
den Projekten aus Karte U43), Snapshot über den Diagnose-Briefkasten
(`snapshot-runde01.json`), Karte
U40 (`u40-aktivitaetsgate.json`, Aktivitätsgate nach NAK-283 Etappe 5) und die
Fenstererfassung ohne Vordergrundwechsel (`fenster.json`).

Format und Erwartungsarten stehen im Kopf von `tools/fl/szenario.py`.
Aktionen sind die des Controller-Skripts
(`C:\Users\phili\Projekte\fl-studio-mcp\fl_controller\device_FLStudioMCP.py`,
Funktion `dispatch_command`). Pegel: `peak_max` 0 = Stille, 1 = 0 dBFS.
