# Entscheid 24.08.2026 — Nakama-Design ist immer die einzige aktuelle Figma-Quelle

## Wörtlicher User-Entscheid

Zum Figma
[`Nakama-Design`](https://www.figma.com/design/NPCQYSkoZEd4Av0NlKxBOd/Nakama-Design?node-id=0-1&p=f&t=KqmMHaY5nyhATZtu-0),
Key `NPCQYSkoZEd4Av0NlKxBOd`:

> „hier sind die aktuellsten varianten. die sind IMMER in diesem figma,
> nirgends wo anders. hier ist immer das aktuellste“

## Was damit festliegt

1. `Nakama-Design` ist nicht nur das spätere Übergabedepot, sondern **zu jedem
   Zeitpunkt die einzige Quelle des aktuellen Designstands**.
2. Andere Figma-Dateien, Kopien, Klone und deren Node-IDs sind ausschließlich
   Verlauf. Sie dürfen weder als aktuelle Arbeitsquelle noch als stiller
   Zwischenstand für die Implementierung behandelt werden.
3. Ein neuer Figma-Stand gilt erst dann als aktuell, wenn er in
   `Nakama-Design` liegt. Es gibt keinen nachträglichen „Promote aus der Copy“-Pfad.
4. Repo-Exporte und Web-Goldens sind datierte Snapshots. Sie beweisen, woraus
   eine Implementierung gebaut wurde, ersetzen aber nie den Live-Stand im
   verbindlichen Figma.

## Aktuelles Live-Inventar nach der EQ-Korrektur

Am 24.08.2026 direkt per Figma-API gelesen:

| Inhalt | Wrapper | innerer Frame | Maß |
|---|---:|---:|---:|
| Gen Seite 1 / Overview | `25:443` | `25:444` | 760×430 |
| Gen Seite 2 / EQ Center | `28:993` | `28:994` | 760×430 |
| Gen Seite 1 / Zustände | `25:805` | — | 960×580 Tafel |
| Gen Seite 2 / Zustände | `25:901` | — | 1120×620 Tafel |

Die vier Bereiche enthalten beim erneuten Prüfzeitpunkt zusammen 554 Nodes,
aber keine Figma-Reaktionen und keine
Component-/Component-Set-/Instance-Nodes. Das ist eine Zustandsfeststellung,
keine Abnahme des Gesamtdesigns.

## Nachtrag nach der Korrektur

> „habe eq center aktualisiert, war ein fehler. ist jetzt das aktuelle drin
> mit 430“

Damit ist die Größenabweichung im einzig verbindlichen `Nakama-Design`
geschlossen. Der vorherige EQ-Stand `25:583`/`25:584` mit 700×420 ist
Verlauf; aktuell sind `28:993`/`28:994` mit 760×430. Die Korrektur
bedeutet noch keine Abnahme der gesamten Optik, Komponentenvarianten oder
Motion.
