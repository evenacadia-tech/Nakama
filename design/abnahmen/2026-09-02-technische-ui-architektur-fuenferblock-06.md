# 2026-09-02 — Technische UI-Architektur, Fünferblock 06

## Status

**1 von 5 Entscheidungen gesammelt.** Der Block begann mit Gen Fläche 1
(Karte U27 aus `docs/plan/fragen.json`), gestellt aus dem Dirigentenlauf vor
dem Phasengate G3, weil der Plantext des Gates die P3-Diagnoseentscheidung aus
Register NAK-57 als vorher fällig nannte. Skizze, Detailabnahmen und Blueprint
werden nach der fünften Antwort gemeinsam umgebaut (Arbeitsmodus vom
31.08.2026); der Blueprint trägt den Entscheid schon jetzt in Abschnitt 15,
Punkt 4, weil er dort als offener Punkt geführt war. Bindend ist der Wortlaut
unten, nicht diese Zusammenfassung.

## 1/5 — Keine Messlücken-Zähler auf Gen Fläche 1 (U27)

Gefragt wurde, welche der übrigen Messlücken-Zähler aus Register NAK-57 auf
Gens erster Fläche einen sichtbaren Platz bekommen: Blöcke, die bei Überlast
verworfen wurden; zu große Blöcke; Zeitsprünge des Musikprogramms; eigene
Analyse-Lücken; Blöcke in der Quarantäne; die größte je gelieferte Blockgröße.
Sichtbar sind heute nur „verworfene Frames“ und, seit U17.5 (30.08.2026), die
Lautheit mit ihrer Unsicherheit je Quelle. Drei Wege standen zur Wahl: (a) ein
Hinweis „Messung hatte Lücken“ je Quelle, Ursachen erst beim Öffnen
(empfohlen); (b) nichts auf der ersten Fläche, nur eine getrennte
Diagnoseansicht; (c) alle Zähler einzeln je Quelle. Der User antwortet am
02.09.2026 um 13:41 Uhr:

> „WAS SOLL der user dann damit anfangen ? das nützt ihm garnichts, also muss er davon nichts sehen. das ist ein audio tool kein tool für den entwickler der app“

Damit ist für Gen Fläche 1 gesammelt:

- **Keiner dieser Zähler bekommt eine Fläche.** Kein Hinweis „Messung hatte
  Lücken“, kein Zähler je Quelle, und auch keine getrennte Diagnoseansicht als
  Produktfläche: Weg (b) ist nur in seiner klaren Form gewählt — die Zahlen
  bleiben Entwicklerdiagnose in den Beweisläufen (Beine B4, B5, B9) und in
  Beweisausgaben.
- **Sichtbar bleiben, unverändert:** Frische je Quelle (SONDE-012), Lautheit
  mit Unsicherheit (U17.5, 30.08.2026) und die Capability-Degradation am
  Zielumschalter (Blueprint Abschnitt 6). Die Deutung im Register, Drop- und
  Bruchzähler seien die in Bauaufteilung §0.4/§1.4 geforderte Degradation,
  ist überholt.
- **Leitsatz,** in Fortsetzung von Block 03 („weniger zeigen“): Nakama ist ein
  Audiowerkzeug für den Musiker; Entwicklerdiagnose gehört nicht auf die
  Oberfläche.
- **Folgen:** Register NAK-57 per User-Entscheid geschlossen; NAK-56 (Karte
  U13) trägt den Nachtrag, dass beim FL-Kurztermin der Beweislauf die
  getrennten Ursachen liest, nicht Gen; Blueprint Abschnitt 15 Punkt 4 steht
  auf [D]; der Plantext von G3 (`docs/plan/plan.json`) trägt den Nachtrag, dass
  vor dem Gate nichts mehr fällig ist.
- **Technische Grenze, vorher geklärt:** Die Zähler existieren im Code
  (`eq-copilot/plugin/sonde/SondeProcessor.h`, `eq-copilot/plugin/core/analysis/FeatureEngine.h`,
  `eq-copilot/plugin/src/PluginProcessor.h`) und werden von den Beinen B4, B5
  und B9 gelesen. Der Entscheid ändert keinen Code; er verhindert eine Anzeige.

## Offen in diesem Block

Die nächste Frage entsteht erst aus einer echten offenen Architekturfrage
einer der drei Flächen; der Block wird nicht mit erfundenen Fragen aufgefüllt.
