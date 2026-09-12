# docs/archiv — historische Dokumente, keine Vorgabe

Verschoben am 21.08.2026 (Kontext-Inventur). Jedes Dokument trägt im Kopf
seinen Status. Die gültige Produktdefinition steht in `CLAUDE.md`
(Wahrheitskern + Entscheide-Register); der technische Entwurf ist
`docs/FL-Nakama-Sonden-Design-Entwurf.md`.

| Datei | Was es war | Warum Archiv |
|---|---|---|
| `FL-EQ-Copilot-Recherche.md` | Produkt-/Umsetzungsplan 12.–16.08. („kanonischer Plan") | User 21.08.: „Archiv". Kennt weder Sondenfamilie noch eigenständigen Broker; führt Tauri-Hub-App und Claude-Schicht — beides gestrichen. Die DSP-/Realtime-Regeln daraus leben in CLAUDE.md und im Code. |
| `FL-EQ-Copilot-Mockup.html` | Klick-Mockup „Probefahrt R3", UI-Generation 1 (13.08.) | Designhistorie |
| `FL-EQ-Copilot-Plan.md`, `leitstand-karte-eq-copilot.md` | Kurzplan / Leitstand-Karte (Hub-App-Zeit) | tote Links, Hub-App gestrichen |
| `fl-claudemd-auszug-2026-08-18.md` | Auszug der FL-Studio-CLAUDE.md vom Umzugstag | Präsens darin („AKTUELLER STAND TIEFENFELD", „NÄCHSTE SESSION: Design-Freeze") ist historisch |
| `NAKAMA-SPECTRAL-FIELD-BAUPLAN.md` | Bauplan 2.0 „Spectral Field" für die Hauptansicht | „Verbindlich" war Claude-Text ohne User-Zitat; User 21.08.: „alle alten sind alt" |
| `NAKAMA-PAPIER-3D-EFFEKTE-PLAN.md` | Papier-3D-Effekte E0–E5 | verworfen 16.08. („das sieht sehr schlecht aus") |
| `EQ-COPILOT-TECHNIK-UX-UEBERGABE.md` | Technik-/UX-Übergabe 14.08. | Hub-App, Tauri, FL-Pfade |
| `ux-ui/` | UX-Arbeitskern + Wissensbasis | Quell-Archiv; die lebende Fassung liegt in `design/docs/` |
| `claude-spike/bare-lauf-2026-08-13.json` | Protokoll des gescheiterten `claude --bare`-Spikes | KI-Erklärschicht vom User 21.08. aus dem Produkt gestrichen |
| `CLAUDE-kontext-vor-bereinigung-2026-08-24.md` | Snapshot der Daueranweisung vor der Bereinigung am 24.08.2026 | nur Spurensuche (CLAUDE.md verweist darauf) |
| `FL-Nakama-Sonden-Design-Entwurf-0.4-2026-08-20.md` | Fassung 0.4 des Entwurfs, byteidentisch eingefroren | Fassung 0.5 ist der lebende Entwurf; die toten relativen Links darin bleiben wegen der Byteidentität |
| `hub-2026-08-23.json` | letzter Stand des Hub-Status vor der Abschaffung (`docs/plan/LIES-MICH.md`) | Planstand wird seit 23.08.2026 gerechnet |
| `vorhaben-2026-08-25-dirigent-ohne-nimbalyst.md` | Stufenplan vom 25.08.2026, den Dirigenten ohne Nimbalyst zu bauen (Stufe C und D nie begonnen) | verschoben 12.09.2026; Nimbalyst ist seit 26.08.2026 deinstalliert, der Dirigent läuft über `.claude/skills/dirigent/` und `tools/dirigent/` |
| `dirigent-protokoll-2026-08-24.md` | Sessionprotokoll des Dirigenten bis 24.08.2026 (vorher `docs/dirigent/protokoll.md`) | verschoben 12.09.2026; Übergaben stehen seither in Manifest, Planstand und Register, nie in einem Sessionprotokoll |
| `2026-08-27-sondenplan-ki-code-qualitaetsaudit.md` | Gemini-Audit des Sondenplans vom 27.08.2026 (vorher `docs/reviews/`) | verschoben 12.09.2026; nie ausgewertet, alle Quellenlinks zeigen auf einen `Documents`-Pfad, den es nicht gibt |
| `ultracode-2026-08-27-sondenplan-debug/` | Paketplan und Verifikation des Ultracode-Laufs vom 27.08.2026 (vorher `.workflow/ultracode/…`, ein gitignorierter Werkzeugordner) | verschoben 12.09.2026; sieben `wissen/`-Blätter zitieren die Pakete als Quelle |

Nicht hier, sondern in `eq-copilot/design/prisma-studie/docs/`: `design-stand.md`,
`geschmacksprofil.md`, `visuelles-zielbild-hoerkompass.md`, `spielregeln-skizze.html`,
`2026-08-19-stmap-live-refraktion.md` — sie gehören zur geparkten Studie.
