# 2026-09-01 — Technische UI-Architektur, Fünferblock 02

## Status

**5 von 5 Entscheidungen abgeschlossen und integriert.** Skizze,
Detailabnahme, Verhaltensvertrag und Blueprint wurden nach Entscheidung 5
gemeinsam aktualisiert. Die technische Skizze wurde anschließend im internen
Browser sichtbar geprüft.

Für die restlichen Fragen dieses Blocks delegiert der User die Ableitung am
01.09.2026 ausdrücklich an die belegte Produkterfahrung vergleichbarer EQs:

> „bitte schaue dir die manuals von TDR Nova FABFILTER und bekannten eq an und
> leite sinnvolle UX freundliche methoden ab zu den weiteren fragen, habe keine
> zeit mehr“

Die Entscheidungen 3 bis 5 sind deshalb keine nachträglich erfundenen
Geschmacksentscheidungen. Sie übertragen wiederkehrende Bedienmuster aus den
offiziellen Handbüchern nur dort, wo sie mit Nakamas aktuellem
Parametervertrag, der festen Bandidentität und den bereits abgenommenen
Panelregeln vereinbar sind.

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

## 3/5 — Bandaktivierung bleibt ein stabiler Panel-Control

Aus TDR Nova, FabFilter Pro-Q 4 und Kirchhoff-EQ ergibt sich eine gemeinsame
Grenze: Bandaktivierung wird von Filtertyp, Dynamic-Aktivierung und Löschen
getrennt. Für Nakama liegt der kompakte `enabled`-Control dauerhaft im Kopf des
objektgebundenen Band-Panels und bleibt in Grund-, Typ-, Kanal- und
Dynamic-Ansicht am selben Ort.

Damit gilt:

- Der Control zeigt den vertraglichen Bandzustand `enabled` als `ON` oder
  `OFF` und ändert ausschließlich dieses Bool-Feld am Blockrand.
- Ausschalten setzt weder Filtertyp noch `channel_mode`, `dynamic_enabled`
  oder einen Zahlenwert zurück. Es ist ein reversibler Bypass desselben festen
  Band-Slots.
- Der Einstieg ist mit Zeiger und Tastatur erreichbar und besitzt einen
  eindeutigen zugänglichen Namen. Eine versteckte Modifikatortaste darf nicht
  der einzige Ausschaltweg sein.
- Der Dynamic-Control bleibt semantisch getrennt: `DYN · OFF` bedeutet
  ausschließlich, dass `dynamic_enabled` aus ist; es sagt nichts über
  `enabled` des gesamten Bands aus.

## 4/5 — Ein ausgeschaltetes Band bleibt direkt wieder erreichbar

FabFilter Pro-Q 4 und Kirchhoff-EQ lassen ein umgangenes Band als inaktives
Objekt an seiner Position bestehen. TDR Nova kann inaktive Bänder dagegen aus
dem Graphen nehmen, bietet dafür aber eine permanente separate
Band-Aktivierungszeile. Eine solche Zeile ist in Nakamas graphdominanter
Architektur bewusst nicht vorhanden. Deshalb darf Nakama die einzige direkte
Rückkehrstelle nicht verstecken.

Damit gilt:

- Ein mit `enabled = false` ausgeschaltetes, aber belegtes Band bleibt als
  eindeutig inaktiver Bandpunkt samt Kurvenbezug sichtbar und auswählbar.
  Der Zustand darf nicht nur durch Farbe vermittelt werden.
- Das geöffnete Panel bleibt beim Ausschalten offen. Sein `OFF`-Control bleibt
  fokussiert; Parameter und Unteransichten dürfen weiterhin inspiziert und
  bearbeitet werden, ohne das Band dadurch still einzuschalten.
- Erneute Aktivierung verwendet denselben Slot, dieselben Werte und dieselbe
  Band-ID. Ausschalten gibt keinen der acht Plätze frei.
- Nur ein ausdrücklich separates Entfernen darf den Bandpunkt aus der
  Arbeitsfläche nehmen. `enabled = false` und „Slot frei“ sind damit zwei
  verschiedene Zustände.

## 5/5 — Entfernen ist getrennt, bewusst und wiederherstellbar

FabFilter Pro-Q 4 und Kirchhoff-EQ führen Bypass und Delete/Remove als getrennte
Bandaktionen. Pro-Q stellt entfernte Bänder über den stabilen Undo-Verlauf
wieder her. Für Nakamas feste acht Automations-Slots ist diese Trennung
zwingend: Nur Entfernen darf einen Platz freigeben; ein Bypass darf niemals
still die Identität des Slots neu belegen lassen.

Damit gilt:

- `Remove Band` sitzt als eigene kompakte Aktion am Rand des Panelkopfs,
  getrennt von `ON/OFF`, Dynamic und dem Schließen-Control. Schließen entfernt
  nichts; Ausschalten entfernt nichts.
- Entfernen ist eine atomare UI-Transaktion: Der belegte Slot wird frei, das
  Panel schließt und der Graph zeigt die entfernte Band-ID nicht mehr. Andere
  Slot-IDs werden niemals umnummeriert.
- Direkt nach dem Entfernen steht ein eindeutiger Undo-Rückweg bereit. Undo
  stellt Slot-ID, Typ, Channelmode, Dynamic-Zustand und sämtliche Werte als ein
  Objekt wieder her.
- Die sichtbare Aktion darf nicht nur als Doppel-/Alt-Geste existieren. Solche
  Expertengesten können später ergänzen, aber niemals den auffindbaren und
  zugänglichen Hauptweg ersetzen.

Der heutige 109-Parametervertrag besitzt `enabled`, aber kein separates
persistentes `occupied`- oder Remove-Feld. Die Skizze belegt deshalb das
Zielverhalten, während die native Umsetzung bis zur versionierten
Slotfreigabe-, Transaktions- und Undo-Semantik technisch blockiert bleibt.
Ein sichtbarer produktiver Remove-Control ohne diesen Vertrag wäre unehrlich.

## Quellenledger der delegierten Ableitung

- [TDR Nova Manual — Main Display und Band Selection/Activation](https://docs.tokyodawn.net/nova-manual/): direkter Graphzugriff, getrennte Auswahl- und Aktivierungszeile sowie Deaktivierung am Bandpunkt.
- [FabFilter Pro-Q 4 Manual — Band Controls](https://www.fabfilter.com/help/pro-q/using/bandcontrols): schwebende Controls am ausgewählten Band, getrennte Bypass-, Delete- und Stereo-Placement-Aktionen.
- [FabFilter Pro-Q 4 Manual — EQ Display](https://www.fabfilter.com/help/pro-q/using/eqdisplay): objektnahe Auswahl, Bypass und Delete; Modifikatortasten nur als zusätzlicher Schnellweg.
- [FabFilter Pro-Q 4 Manual — Undo and Redo](https://www.fabfilter.com/help/pro-q/using/undoredo): wiederherstellbare UI-Änderungen an einem stabilen Undo-Ort.
- [Kirchhoff-EQ Manual](https://files.plugin-alliance.com/products/tbt_kirchhoff-eq/tbt_kirchhoff-eq_manual.pdf): separates Enable/Disable, Dynamic, Stereo Mode und Remove im Bandkontext.

## Integrationsbeleg

- Der aktuelle Parametervertrag und seine C++-Tabelle wurden erneut gegen
  `enabled`, `channel_mode`, `dynamic_enabled` und die festen acht Slots
  geprüft;
  `tools/codex-plugins/audio-plugin-ux/skills/audio-plugin-ux/scripts/check-nakama-contract.ps1`
  meldete am 01.09.2026 `ok: true` und `failedCount: 0`.
- Die technische Skizze zeigt Panel-Rebinding, einen dauerhaft stabilen
  `ON/OFF`-Control, die fünf Kanalmodi im selben Panelkörper, einen sichtbar
  deaktivierten aber weiterhin erreichbaren Bandpunkt sowie getrenntes Remove
  mit Undo-Wiederherstellung.
- Die Skizze wurde im internen Browser am bestehenden lokalen URL sichtbar
  geprüft. Ein oberer freier Graph-Doppelklick erzeugte B7 und platzierte das
  276 px breite Panel darunter; ein unterer erzeugte B8 und platzierte es
  darüber. Beide Zustände blieben vollständig in der Graphfläche. Die
  Messungen an B1 und B6 bestätigten zusätzlich die linke und rechte
  Kantenbegrenzung.
- Browserinteraktionen bestätigten: Einfachklick B3 → B5 bindet das geöffnete
  Panel samt korrekten B5-Werten neu; die Kanalansicht zeigt genau fünf Modi
  und kehrt nach der Wahl zurück; `OFF` bleibt in dieser Unteransicht stabil;
  Remove B2 schließt das Panel und entfernt nur B2; Undo stellt B2 mit
  `Left`, Bypass und seinen bisherigen Werten wieder her. Die Browserkonsole
  meldete dabei keine Fehler.
