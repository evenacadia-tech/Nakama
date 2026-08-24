# Entscheide 24.08.2026 — Undo, Sidechain und Probeeq-Rückfallfläche

## Anlass

Die noch als offen geführten Figma-Fragen wurden gegen die ausführlichen
Interviews vom 22. und 23.08. abgeglichen. Bereits beantwortete Grundfragen zu
Vergleich, PROTECTED, Mix und Rückfallinhalt wurden nicht erneut gestellt.

## 1. Undo auf Gen-Seite 1

Wahl des Users: **kleiner Revisionsverlauf**.

Folge:

- kein großer Undo-Button;
- kein bloßer Einsatz `UNDO · n` ohne nachvollziehbaren Inhalt;
- ein kleines Verlaufssymbol öffnet die bestätigten Änderungen;
- Draft, gehaltene Vorschau und abgelaufene temporäre Anwendung erzeugen
  keinen Eintrag.

Das kleine sondeneigene Undo-Symbol auf Seite 2 bleibt unverändert der
Entscheid vom 23.08.

## 2. Sidechain-Quelle je Band

Wahl des Users: **im ersten Release unsichtbar**.

Folge:

- kein dreizehntes sichtbares Feld in der Bandzeile;
- kein DYN-Untermenü für die Sidechain-Quelle im ersten Release;
- der gespeicherte Parameter bleibt vorhanden und steht standardmäßig auf
  `none`;
- eine sichtbare Bedienung kommt erst mit einem späteren, technisch
  freigegebenen Sidechain-Schritt.

Damit ist die reine UI-Frage U5/NAK-33 beantwortet. Die getrennten technischen
Sidechain-/PDC-Gates bleiben davon unberührt.

## 3. Probeeq-Rückfallfläche

Wörtliche Antwort:

> „wir haben schon ein entwurrf er ist auch in dem figma design workspace.
> heißt Suna, schau es dir an in figma“

Die einzige aktuelle Designquelle bleibt
[`Nakama-Design`](https://www.figma.com/design/NPCQYSkoZEd4Av0NlKxBOd/Nakama-Design?node-id=0-1).
Der Live-Check fand dort:

- Workspace `Suna`: Node `6:2863`, 900×1120;
- eigentlicher Entwurf `Suna `: Node `6:2864`, 260×84.

Der sichtbare Entwurf trägt derzeit PASSIVE PROBE, Bus, Rolle, Position,
CONNECTED, MEASURING und die Wortmarke NAKAMA SUNA. Er ist die verbindliche
visuelle Ausgangsbasis für die minimale Probeeq-Rückfallfläche und wird nicht
durch einen frei erfundenen neuen Stil ersetzt.

Der notwendige Umbau folgt den bereits festgelegten Produktinhalten:
Probeeq-Name, Connection-Ausnahmen, EQ-Mode, geänderte EQ-Werte, Bypass und
später Mix. Welche vorhandenen Suna-Felder dafür entfallen oder umgenutzt
werden, wird am Entwurf sichtbar ausgearbeitet und anschließend abgenommen.
