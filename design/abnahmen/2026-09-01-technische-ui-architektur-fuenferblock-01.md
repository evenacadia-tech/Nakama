# 2026-09-01 — Technische UI-Architektur, Fünferblock 01

## Status

**4 von 5 Entscheidungen gesammelt.** Der gemeinsame Umbau, die vollständige
Sichtprüfung und die Integration in die Detailabnahmen sowie den Blueprint
folgen nach Entscheidung 5.

Für diesen Block gilt der neue User-Entscheid:

> „WÄRE ES nicht sinnvoller wir sammeln wieder ein paar entscheidungen? nach
> jeder frage direkt umbauen ist doch sehr umständlich ? wir können das fest
> machen. immer 5 fragen dann umbauen“

## 1/5 — Dynamic beim ersten Öffnen aktivieren

Auf die unmittelbar zuvor gestellte Architekturfrage wählt der User:

> „C“

Damit ist für Gen Fläche 2 gesammelt:

- Ein Klick auf `DYN · OFF` setzt `dynamic_enabled` für das ausgewählte Band
  auf `true` und öffnet im selben Handgriff die bereits entschiedene
  zweireihige Dynamic-Werteansicht.
- Ein Klick auf `DYN · ON` öffnet beziehungsweise verlässt nur diese Ansicht;
  er schaltet den Klangzustand nicht aus.
- Das Verlassen oder Schließen der Werteansicht lässt Dynamic aktiv.
- Der ausdrückliche Bedienort zum späteren Ausschalten bleibt eine echte
  offene Architekturfrage dieses Blocks.

Technisch bleiben `dynamic_enabled` und die fünf Werteparameter getrennte
persistente Parameter. Ein späteres Ausschalten darf `dynamic_range_db`,
`threshold_db`, `attack_ms`, `hold_ms` und `release_ms` daher nicht still auf
Defaults zurücksetzen.

Vor Eingang der Taktänderung war dieser erste Übergang in der laufenden Skizze
bereits begonnen und an einem zuvor inaktiven Band geprüft worden. Der
kohärente Fünfer-Umbau samt vollständigem Sichtbeleg und Repo-Integration bleibt
trotzdem bis 5/5 zurückgestellt.

## 2/5 — Ausschalten am Anfang der Dynamic-Ansicht

Auf die Frage nach dem ausdrücklichen Bedienort zum Ausschalten eines aktiven
Dynamic-Bands wählt der User:

> „A“

Damit ist für Gen Fläche 2 gesammelt:

- Am Anfang der geöffneten Dynamic-Werteansicht sitzt ein kompakter
  `dynamic_enabled`-Zustandscontrol für das ausgewählte Band.
- Dieser Control ist der ausdrückliche Ein-/Ausschalter. Der bereits
  entschiedene `DYN`-Control im Panelkopf bleibt Einstieg und Disclosure und
  schaltet ein aktives Dynamic-Band nicht durch bloßes Öffnen oder Verlassen
  der Ansicht aus.
- Ausschalten ändert ausschließlich `dynamic_enabled`; die fünf gespeicherten
  Dynamic-Werte bleiben erhalten.
- Welche Ansicht unmittelbar nach dem Ausschalten bestehen bleibt und wohin
  der Fokus geht, bleibt als nächste Architekturfrage offen.

## 3/5 — Nach dem Ausschalten zu den Grundwerten zurückkehren

Auf die Frage nach Ansicht und Fokus unmittelbar nach dem Ausschalten von
Dynamic wählt der User:

> „A“

Damit ist für Gen Fläche 2 gesammelt:

- Das Ausschalten über den Control am Anfang der Dynamic-Ansicht setzt
  `dynamic_enabled` auf `false`, lässt das objektgebundene Mini-Panel jedoch
  geöffnet.
- Im selben Panelkörper erscheinen wieder `Frequency`, `Gain` und `Q`.
- Der Tastaturfokus geht auf den weiterhin sichtbaren Einstieg `DYN · OFF`.
- Die fünf gespeicherten Dynamic-Werte bleiben unverändert und werden erst
  nach einer erneuten Aktivierung wieder eingeblendet.

## 4/5 — Pointer schließt nur über den eigenen Panel-Control

Auf die Frage nach dem Schließen des Mini-Panels per Zeiger wählt der User:

> „B“

Damit ist für Gen Fläche 2 gesammelt:

- Das Mini-Panel behält einen eigenen, sichtbaren Schließen-Control.
- Ein einfacher Klick auf eine freie Stelle des EQ-Graphen lässt das Panel
  geöffnet und verändert weder Auswahl noch Parameterzustand.
- `Escape` bleibt der technisch festgelegte Tastaturweg: Eine laufende
  Werteingabe wird zuerst abgebrochen; andernfalls schließt es das oberste
  nichtmodale Panel.
- Schließen verändert weder den Bandzustand noch `dynamic_enabled` und gibt
  den Tastaturfokus an den zugehörigen Bandpunkt zurück.
