# Nakama

Plugin-Familie für FL Studio, die beim Mischen berät: **Nakama Gen** (Main),
**Nakama Probeeq** (aktive Sonde, vollwertiger EQ), **Nakama Suna** (passive
Sonde); Bundle **Nakama Studio**. Technik: JUCE 8 / C++20 / CMake +
eigenständiger Rust-Broker (Named Pipe). Code, Bundle und Pipes tragen heute
noch den Legacy-Namen EQ-Copilot (`EqCop*`).

Dieses Repo hält **Code, Verträge, Beweise und technisches Wissen**. Das Design
der drei Apps entsteht in Figma (User) und wird in `Projekte\Nakama-Design`
in lebende Blätter übersetzt.

| Wohin | Wofür |
|---|---|
| `CLAUDE.md` | Wahrheitskern, Register der User-Entscheide (mit Zitat), Bauen & Beweisen, Invarianten |
| `docs/NEXT-SESSION.md` | Einstieg: Stand und der eine nächste Schritt |
| `docs/plugin-wissen.md` | Wie das Plugin heute funktioniert |
| `docs/FL-Nakama-Sonden-Design-Entwurf.md` | Technischer Entwurf der Sondenfamilie (Fassung 0.4 + Errata) |
| `docs/bauaufteilung-sonden.md` | Sessions, Gates, Prüfstufen |
| `docs/beweise/` | Beweismanifeste je Ticket (rohe Ausgabe) |
| `docs/offene-punkte.md` | Offen-Set (nie still löschen) |
| `docs/archiv/` · `eq-copilot/design/archive/` · `eq-copilot/design/prisma-studie/` | Verlauf: alte Pläne, alte Design-Richtungen, geparkte Prisma-Studie — nie Vorgabe |
| `tools/beweise.ps1` | Ein Befehl für den ganzen Beweis-Kanon |
| `tools/hooks/` | SessionStart-Primer (liest den Wahrheitskern aus CLAUDE.md), Handoffs, Guards |

Bauen und Beweisen: siehe `CLAUDE.md`, Abschnitt „Bauen & Beweisen".
