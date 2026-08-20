# 2026-08-20 — Vorhören zeigt sich an der betroffenen Zeile, nicht an der ganzen App

**Abnahme.** Beurteilt an drei bedienbaren Blättern auf 760×430, mit echtem
Halten und Loslassen. Die App-Umrandung war zu diesem Zeitpunkt gebaut und
direkt danebengestellt — sie wurde gesehen, gehalten und verworfen.

## Das wörtliche Urteil

```json
{
 "werkzeug": "nakama-bedienebenen", "version": 1,
 "zeitpunkt": "2026-08-20T19:03:23.051Z",
 "stufe": "2 / Main / die vier Bedienebenen", "oberflaeche": "Main",
 "engereWahl": ["zeile", "beides"],
 "sieger": "zeile",
 "mechanik": "Nur die markierte Zeile",
 "ablehnungen": [],
 "sekunden": { "schritt1": "59.3", "schritt2": "18.4" }
}
```

## Das ist eine bewusste Rücknahme

Eine Stunde zuvor stand:

> „ich wollte die markierte zeile + die app umrandung"

Nachdem beides gebaut, nebeneinandergestellt und **gehalten** wurde, fiel die
Wahl auf die markierte Zeile **allein**. Die Umrandung ist damit verworfen —
nicht ungebaut, sondern gesehen und abgelehnt. Das ist der vorgesehene Weg:
eine Zustandsgestaltung lässt sich am Standbild nicht beurteilen.

## Was damit festliegt

**Vorhören zeigt sich am Ort der Wirkung.** Die betroffene Quelle in der
Karte wird markiert, solange gehalten wird; beim Loslassen ist die Markierung
sofort weg. Kein Zeichen umfasst die ganze App.

Das vollständige Zeichenbild des Vorhörens — drei Zeichen, drei verschiedene
Fragen, keines doppelt:

| Zeichen | Ort | beantwortet |
|---|---|---|
| `AUDITION`-Reiter leuchtet | Streifen, dauerhaft sichtbar | *auf welcher Ebene bin ich?* |
| Griff sichtbar gedrückt | unter dem Finger | *ich tue es gerade* |
| Zeile markiert | in der Karte | *und zwar hier* |

Damit ist auch die offene Frage der letzten Runde beantwortet: **der Reiter
leuchtet weiter.** Er ist jetzt die einzige globale Auskunft und deshalb
nicht verhandelbar — die Spezifikation verlangt „vier Bedienebenen, sichtbar
getrennt".

## Der Preis, benannt und angenommen

Die These der gewählten Variante nennt ihn:

> „wer gerade auf den Assistenten schaut, sieht nicht, dass etwas läuft."

Er ist kleiner, als er klingt, und das gehört zur Begründung: **beim Vorhören
hält die Hand den Griff.** Wer vorhört, weiß es körperlich; das Zeichen muss
ihn nicht daran erinnern, sondern nur den Ort nennen. Eine App-weite
Umrandung wäre für einen Zustand, den man selbst gerade auslöst, ein sehr
lautes Signal — und laut ist teuer, weil es jede Sekunde mitläuft.

## Was ausdrücklich offen bleibt

- **Die Zustände** der Spezifikation: Quelle frisch / veraltet / getrennt,
  Messung läuft / unvollständig, degradierte Funktion. Sie stehen im
  Streifen als Text, sind nicht durchschaltbar und nicht gestaltet. Das ist
  der nächste Schritt für Main.
- **Mehrere angewandte Änderungen.** „1 change applied" ist ein Zähler ohne
  Verlauf; die Spezifikation kennt einen Undo-Verlauf nur für den
  Active-Probe-Editor.
- Ob die Markierung bei **veralteten oder getrennten** Quellen anders
  aussehen muss.
- Alles Visuelle.

## Kein Prüfpunkt ausgelöst

Welt und Fassung bleiben **Arbeitsannahmen**. Das Layout von Main steht auf
Wireframe-Ebene fast vollständig, aber die Zustandsmatrix fehlt.

## Bezug

- [bedienebenen-beides](2026-08-20-bedienebenen-beides.md) — die
  Richtungsabnahme davor, samt meiner falschen ersten Übersetzung
- [karte-alle-quellen](2026-08-20-karte-alle-quellen.md)
- `werkzeug/LIES-MICH.md`, Abschnitt `bedienebenen.html`
