# Arbeitsplan — aktueller Design-Arbeitsweg

Stand 26.08.2026. Dieser Text ist ein Router, keine zweite Produktwahrheit.
Den nächsten Produktauftrag bestimmt `../../docs/PLAN-STAND.md`; für eine
konkrete Designarbeit gelten zuerst `../LIES-MICH.md`, die aktuelle Abnahme
und die betroffene Implementierung.

## Ausgangslage

- Der User ist Projektleiter und gibt die visuelle Richtung grundsätzlich
  vor.
- Designvertrag, Formfaktor, Informationsstruktur, Wireframes, Skizzen und
  Graybox waren frühe Projektschritte und sind abgeschlossen. Sie werden im
  laufenden Rework nicht als Phasen oder Gates wiederholt.
- Nakama hat zwei Apps und drei aktuelle Flächen: Gen Seite 1, Gen Seite 2
  und Probeeq. Suna ist in Probeeq aufgegangen.
- Die aktuelle visuelle Wahrheit in progress sind ausschließlich die drei
  Bilder unter `../assets/rework-basis-2026-08-25/` gemäß
  `../abnahmen/2026-08-25-rework-referenz-drei-designs.md`.
- Frühere Figma-Nodes, Exporte, Hub-Karten und die alte Phasenfolge sind
  Verlauf, keine Arbeitsanweisung.

Die Klarstellung zur abgeschlossenen Anfangsphase steht als Nachtrag in
`../abnahmen/2026-08-25-skizzen-vor-figma.md`.

## Ablauf für einen aktuellen Designauftrag

1. Die betroffene aktuelle Referenzfläche und ihren Abnahmetext lesen.
2. Die betroffene Implementierung und nur die dafür nötigen Produkt- oder
   Interaktionsverträge lesen.
3. Direkt am aktuellen Stand arbeiten. Keine neue Stilrichtung, kein
   allgemeines Interview und keine erneute Wireframe-/Graybox-Runde starten.
4. Zeigt die Referenz ein konkretes Verhalten oder einen Grenzfall nicht,
   genau diese eine Lücke benennen und dem User vorlegen. Bereits
   Entschiedenes nicht erneut öffnen.
5. Das Ergebnis in der festen Zielgröße rendern, ansehen und gegen Referenz,
   Zustände, Grenzfälle und sichtbare Handgriffe prüfen.
6. Das User-Urteil mit Datum und Wortlaut in `../abnahmen/` festhalten.
   Erst eine dokumentierte Abnahme darf in den Produktcode gespiegelt werden.

## Was dieser Plan nicht mehr führt

- keine Phase 0, 1, 1b oder spätere Discovery-Staffel;
- keine Reihenfolge Gen → Probeeq → Suna;
- keine alten Figma-Dateien oder Node-IDs;
- keinen Nakama-Hub und keine Artifact-Antwortschleife;
- keine automatisch wiederkehrenden `/interview`-, Skizzen- oder
  Graybox-Pflichten.

Historische Entscheidungen bleiben in den datierten Abnahmen und in Git
auffindbar. Sie werden nicht in diesen aktiven Arbeitsweg zurückkopiert.
