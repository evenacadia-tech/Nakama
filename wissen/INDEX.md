# Index des Nakama-Wissensarchivs

Stand: 27.08.2026

## Verbindliche bestehende Quellen

| Gegenstand | Kanonischer Besitzer |
|---|---|
| Produktwahrheit und User-Entscheide | [`CLAUDE.md`](../CLAUDE.md) und [`design/abnahmen/`](../design/abnahmen/) |
| Verhalten des heutigen Codes | Quellcode, Tests und [`docs/plugin-wissen.md`](../docs/plugin-wissen.md) |
| Technischer Bauplan | [`docs/FL-Nakama-Sonden-Design-Entwurf.md`](../docs/FL-Nakama-Sonden-Design-Entwurf.md) und [`docs/PLAN-STAND.md`](../docs/PLAN-STAND.md) |
| Sichtbares Design | Figma, freigegebene Goldens und [`DESIGN.md`](../DESIGN.md) |
| Beobachtbares UI-Verhalten | [`UX-CONTRACT.md`](../UX-CONTRACT.md) |
| Historische, nicht mehr bindende Richtungen | [`docs/archiv/`](../docs/archiv/) |
| Eingeordnetes externes Wissen | dieses Verzeichnis |

## Themen

| Bereich | Inhalt | Einstieg |
|---|---|---|
| Produkt | Nutzerproblem, Nutzenversprechen, Begriffe und Produktmodelle | [`produkt/`](produkt/LIES-MICH.md) |
| UX | Aufgaben, mentale Modelle, Abläufe, Fehler, Recovery und Accessibility | [`ux/`](ux/LIES-MICH.md) |
| UI und Motion | visuelle Hierarchie, Controls, Material, Licht und Bewegung | [`ui-motion/`](ui-motion/LIES-MICH.md) |
| DSP | EQ, Messung, Wahrnehmung und Audiobewertung | [`dsp/`](dsp/LIES-MICH.md) |
| Engineering | JUCE, VST3, Echtzeit, Architektur und Tests | [`engineering/`](engineering/LIES-MICH.md) |
| FL Studio | Hostverhalten, Automation, Skalierung und Plugin-Lebenslauf | [`fl-studio/`](fl-studio/LIES-MICH.md) |
| Visuelle Belege | wiederverwendbare Screenshots, Frames, Graphen und Diagramme | [`medien/`](medien/LIES-MICH.md) |

## Neue Einträge

| Datum | Eintrag | Bereich | Prüfstand |
|---|---|---|---|
| 27.08.2026 | [Beweise müssen aus versionierten Quellen neu entstehen](engineering/2026-08-27-beweise-muessen-reproduzierbar-sein.md) | Engineering, DSP, UI/Motion | Reproducible-Builds- und SLSA-Quellen sowie zwei Nakama-Gegenbeweise geprüft |
| 27.08.2026 | [Ein Installer ist eine privilegierte Transaktion](engineering/2026-08-27-installer-als-privilegierte-transaktion.md) | Engineering, FL Studio | Microsoft-Referenz, Installer-Code und Rollback-Beweise geprüft |
| 27.08.2026 | [JSON-Verträge vor dem Parser beweisen](engineering/2026-08-27-json-vertraege-vor-dem-parser.md) | Engineering, FL Studio | RFCs, Code und Mehrsprachenbeweise geprüft |
| 27.08.2026 | [Windows-Pipes sind Sicherheits- und Lebenszyklusverträge](engineering/2026-08-27-windows-pipes-sicherheit-und-lifecycle.md) | Engineering, FL Studio | Microsoft-Referenz, Code und Regressionen geprüft |
| 27.08.2026 | [Crashfester Store und Outbox: mindestens einmal, genau einmal wirksam](engineering/2026-08-27-crashfester-store-und-outbox.md) | Engineering | SQLite- und Outbox-Primärquellen sowie Debugging-Beweise geprüft |
| 27.08.2026 | [Realtime-Qualität misst den seltenen Spike](engineering/2026-08-27-realtime-worst-case-statt-durchschnitt.md) | Engineering, DSP, FL Studio | ADC-Quelle und vollständiger Folientext geprüft; kein visueller Render |
| 27.08.2026 | [Messwerte brauchen Supportintervall und Zeitanker](dsp/2026-08-27-messwerte-brauchen-support-und-zeitanker.md) | DSP, Engineering, FL Studio | Welch, EBU, VST3, Code und Debugging-Beweise geprüft |
| 26.08.2026 | [Zugängliche Audio-Controls müssen auch benutzbar sein](ux/2026-08-26-zugaengliche-audio-controls.md) | UX, UI/Motion, Engineering | Vortrag, W3C und JUCE geprüft |
| 26.08.2026 | [Audio-Interaktion: Kontext statt ungeprüfter Annahmen](ux/2026-08-26-audio-interaktion-kontext-statt-annahmen.md) | UX, UI/Motion, DSP | Videoabschnitte geprüft; kein Transkript verfügbar |
| 26.08.2026 | [Plugin-Modernisierung ohne Vertrauensbruch](fl-studio/2026-08-26-plugin-modernisierung-ohne-vertrauensbruch.md) | FL Studio, Engineering, UX | Quelle und Transkript geprüft |
| 26.08.2026 | [JUCE-UI-Performance messen statt vermuten](engineering/2026-08-26-juce-ui-performance-messen.md) | Engineering, UI/Motion, UX | Quelle, Transkript und JUCE-Referenz geprüft |
| 26.08.2026 | [Visuelle Treue in einer nativen UI-Pipeline](ui-motion/2026-08-26-visuelle-treue-native-ui-pipeline.md) | UI/Motion, Engineering, UX | Folien, Ressourcen und Beschreibung geprüft |
| 26.08.2026 | [AudioDevCon: kuratierte Videoauswahl](engineering/2026-08-26-audiodevcon-videoauswahl.md) | Engineering, UX, UI/Motion, DSP, FL Studio | Auswahl und Metadaten geprüft; Einzelstatus im Eintrag |
| 26.08.2026 | [Nutzerzentriertes Design für Audio-Produkte](ux/2026-08-26-nutzerzentriertes-audio-design.md) | UX | Quelle und Transkript geprüft |
