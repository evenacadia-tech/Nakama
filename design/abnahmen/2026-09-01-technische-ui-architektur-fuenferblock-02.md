# 2026-09-01 — Technische UI-Architektur, Fünferblock 02

## Status

**2 von 5 Entscheidungen gesammelt.** Skizze, Detailabnahmen,
Verhaltensvertrag und Blueprint werden gemäß dem festgelegten Fünfertakt erst
nach Entscheidung 5 gemeinsam aktualisiert und im internen Browser geprüft.

## 1/5 — Geöffnetes Band-Panel folgt der Bandauswahl

Auf die Frage, was mit dem für B3 geöffneten Mini-Panel geschieht, wenn der
User B5 einmal auswählt, wählt der User:

> „a“

Damit ist für Gen Fläche 2 gesammelt:

- Es existiert weiterhin genau ein objektgebundenes Band-Panel. Bei Auswahl
  eines anderen vorhandenen Bandpunkts bindet es sich atomar an das neu
  ausgewählte Band und verankert sich dort neu; es bleibt kein zweites Panel
  am vorherigen Band zurück.
- Nach dem Wechsel zeigt der Panelkörper die Grundansicht des neuen Bands mit
  dessen Filter Type, Frequency, Gain, Q und Dynamic-Zustand. Eine zuvor
  geöffnete Typ- oder Dynamic-Unteransicht wird nicht auf das neue Band
  übertragen.
- Kein Wert und kein Zustandsbit wandert von B3 nach B5. Insbesondere wird
  `dynamic_enabled` des neuen Bands durch die Auswahl weder aktiviert noch
  deaktiviert; B3 bleibt unverändert.
- Das Panel darf nach dem Rebinding ausschließlich auf den neuen eindeutigen
  Bandbesitzer schreiben. Eine sichtbare Auswahl B5 bei weiterhin auf B3
  zielender Eingabe ist ausgeschlossen.

Fokusdetails des Auswahlwechsels werden aus Eingabemodus und
Zugänglichkeitsvertrag abgeleitet; sie sind nicht Teil dieses Entscheids.

## 2/5 — Kanalmodus bleibt in der Grundansicht sichtbar

Auf die Frage, wie der bandlokale Kanalmodus `Stereo`, `Left`, `Right`, `Mid`
oder `Side` im kompakten Mini-Panel zugänglich wird, wählt der User:

> „a“

Damit ist für Gen Fläche 2 gesammelt:

- Der aktuelle Kanalmodus bleibt als kompakter, bandlokaler Zustandscontrol
  in der Grundansicht des Mini-Panels sichtbar.
- Seine Aktivierung ersetzt innerhalb desselben Panelkörpers vorübergehend
  `Frequency`, `Gain` und `Q` durch genau die fünf vertraglichen Modi. Es
  entsteht weder ein zweites Popup noch eine dauerhaft breitere
  Parameteransicht.
- Eine Auswahl ändert ausschließlich `channel_mode` desselben festen
  Band-Slots. Danach kehrt das Panel zur Grundansicht dieses Bands zurück;
  der aktuelle Modus bleibt dort ablesbar.
- Beim Wechsel zu einem anderen Band zeigt der Control dessen eigenen
  Kanalmodus. Ein Modus wird weder vom vorherigen Band übernommen noch mit
  Filtertyp oder Dynamic-Zustand gekoppelt.

Technisch ist `channel_mode` ein diskreter, topologischer Blockrandparameter
mit Default `stereo`; Freitext, Mehrfachwahl und Zwischenzustände sind daher
ausgeschlossen. Englische Namen, aktueller Wert und Auswahlzustand bleiben
über die zugängliche Control-Semantik eindeutig. Die spätere visuelle
Kurzform ist nicht Teil dieses Entscheids.
