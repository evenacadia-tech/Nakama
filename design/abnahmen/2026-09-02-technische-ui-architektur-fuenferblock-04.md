# 2026-09-02 — Technische UI-Architektur, Fünferblock 04 (delegierte Ableitung)

## Status

**Auftrag erteilt, Ableitung läuft.** Der User delegiert die restlichen
offenen UX-Entscheide am 02.09.2026 ausdrücklich an eine belegte Ableitung
aus Handbüchern und Bedienprinzipien:

> „ich habe gerade keine zeit, kannst du nicht die restlichen offenen UX
> entscheidungen anhand von best practice beispielen lösen ? es gibt ja
> manuals , handbücher mit denen du gute menschliche Bedienprinzipien super
> nachvollziehen und anwenden kannst“

Das ist dieselbe Form wie die Entscheidungen 3 bis 5 von Fünferblock 02
(01.09.2026): Übertragbar sind wiederkehrende Bedienprinzipien aus
offiziellen Herstellerhandbüchern und anerkannten Bedienmustern, nicht
Farben, Material, Maße oder die Markenoptik der Referenzprodukte. Jede
Entscheidung unten nennt ihre Quelle; was keine Quelle deckt, bleibt offen
und wird nicht erfunden. Bindend bleiben nur die datierten Abnahmen; diese
Datei ist eine davon, weil sie auf dem ausdrücklichen User-Auftrag beruht.

Weil der User „die restlichen“ Entscheide gemeinsam abgibt, werden Block 04
und der anschließende Block 05 mit einem gemeinsamen Umbau von Skizze,
Abnahmen und Blueprint abgeschlossen; das ist der im Arbeitsmodus
vorgesehene ausdrückliche Userauftrag für eine Abweichung vom Fünfer-Takt.

## Gegenstand: die offenen UX-Entscheide (Stand nach Block 03)

Aus Blueprint Abschnitt 15 und dem Skizzen-Leseblatt, nur Bedienfragen,
keine technischen Vertragslücken:

1. Gen Fläche 2: Disclosure von Voreinstellungen und Verlauf innerhalb der
   Global-Sektion; Ort der Kurven-Kopie (Antwort U18).
2. Gen beide Flächen: Wortlaut der Halten-Aktion und Kurzform `DEL`.
3. Probeeq: Fensterhöhe bei ein- und ausgeblendeter zweiter Zeile.
4. Probeeq: Verhalten bei Link-Verlust und lokale Notfallaktionen.
5. Gen Fläche 1: Sources-Spalte bei 16 sichtbaren und 32 angeschlossenen
   Quellen (Dichte, Scroll, Suche/Quick-Jump).

Block 05 nimmt die gemeinsamen Regeln: Tastatur, Fokus und Radbesitz über
beide Flächen; Fehler- und Degradationszustände.

Nicht Gegenstand, weil technische Verträge und keine Bedienfragen:
Live-Dynamic-Telemetrie, Remove/Undo-Vertrag, 10-s-Kandidatenvertrag,
direkte Edit-Transaktion, Hold-Accessibility in JUCE, Produktnamen der
Flächen (Karte U23, liegt beim User).

## 1/5 — Voreinstellungen im selben Körper, Verlauf ohne Liste, Kopie am Zielwechsler

**Belegte Muster.** FabFilter Pro-Q 4 zeigt den Namen der aktuellen
Voreinstellung auf einem Knopf; wird ein Parameter geändert, „the name is
dimmed to indicate that this is not the original preset anymore“. Ein Klick
öffnet einen Browser mit Ordnern, in dem „you can start typing to search
through the presets“; Pfeile neben dem Knopf „explore presets one by one“;
Sichern läuft über „Save As“ mit Dialog, und Überschreiben fragt nach: „You
will be asked for confirmation before saving.“ Undo und Redo sind zwei
Knöpfe; „There is no dedicated history list“, und der A/B-Knopf hat einen
„Copy“-Knopf, der „the active state to the inactive state“ kopiert.
Kirchhoff-EQ hält den „Preset Selector … at the center of bottom bar“,
Undo/Redo als „two buttons“ und A/B mit „Copy“ gleich. TDR Nova nennt
„preset manager, undo/redo, A/B“. Das W3C-Muster *Combobox mit
List-Autocomplete* beschreibt die Tastatur einer filterbaren Liste (Tippen
filtert, Pfeil ab in die Liste, Enter übernimmt, Escape schließt).

**Entscheidung für Gen Fläche 2 (Global-Sektion):**

- `PRESET` ist ein Knopf fester Maße, der den Namen der aktuellen
  Voreinstellung trägt und bei jeder Abweichung sichtbar als geändert
  markiert ist (gedimmt plus Marker, nicht nur Farbe). Links und rechts
  davon liegen `‹` und `›` für die nächste beziehungsweise vorherige
  Voreinstellung.
- Seine Aktivierung öffnet die Liste **im selben Körper der aufgeklappten
  Global-Sektion**: Ein Filterfeld oben (Tippen filtert, Enter lädt den
  ersten Treffer, Escape schließt zurück zur Sektion), darunter die
  Voreinstellungen in zwei Gruppen (mitgeliefert, eigene). Es entsteht kein
  zweites Popup, so wie Typ- und Kanalwahl im Band-Panel dieselbe Fläche
  nutzen.
- `SAVE AS` sichert unter neuem Namen; `SAVE` überschreibt die geladene
  eigene Voreinstellung erst nach einer sichtbaren Bestätigung im selben
  Körper. Mitgelieferte Voreinstellungen sind nicht überschreibbar.
- Ein Verlauf als Liste wird **nicht** gebaut. Der Verlauf ist bedienbar
  über `UNDO` und `REDO` in der unteren Zeile (Redo ist neu, neben Undo,
  ohne Wirkung ausgegraut) und lesbar über die bestätigte Revision `REV n`
  in der Global-Sektion. Der Knopf `HISTORY` entfällt.
- Die Kurven-Kopie (Antwort U18: gehört dorthin, wo die Sonden
  durchgeschaltet werden) sitzt als `COPY →` am Ende der Zielleiste. Sie
  ersetzt die Zielleiste vorübergehend durch die Zielwahl „copy *Quelle* to:
  *Ziel*“ und wirkt erst nach einer Bestätigung, weil sie den Klangzustand
  der Zielsonde überschreibt (Pro-Q-Muster der Überschreib-Bestätigung;
  Nielsen, Heuristik 5 „Error Prevention“). Quelle und Ziel stehen vor der
  Wirkung im Wortlaut.
- Was Voreinstellung und Kopie tragen, bleibt der Entscheid vom
  30.08.2026: Klanginhalt, nie Identität.

## 2/5 — Ein Wortlaut für das Halten, `DEL` mit Wort daneben

**Belegte Muster.** Pro-Q 4 nennt das Vorhören eines Bands „Solo“ und
bedient es als „momentary control — the feature remains active only while
the button is held down“. Kirchhoff-EQ hat einen „Headphone button: turn
on/off monitoring selected band“ und einen „Remove button: delete selected
band“; Pro-Q einen „delete button … removes the selected EQ bands“ mit
Undo. Nielsen, Heuristik 4: „Users should not have to wonder whether
different words, situations, or actions mean the same thing.“ NN/g zu
Icons: „a text label must be present alongside an icon to clarify its
meaning“, und Abkürzungen sind Wörter, keine Icons.

**Entscheidung für beide Gen-Flächen:**

- Die Halten-Aktion heißt auf beiden Flächen gleich: `HOLD TO AUDITION`
  (Wortlaut der Abnahme vom 22.08.2026). Fläche 2 übernimmt ihn; die
  Transaktionszeile bekommt dafür die nötige Breite, der Knopf bleibt ein
  Material fester Maße. Gehalten zeigt der Knopf `RELEASE TO RETURN` auf
  beiden Flächen.
- `DEL` bleibt die sichtbare Kurzform im Panelkopf, weil dort 276 px sieben
  Handgriffe tragen; sie ist ein Wort, kein Icon. Der volle Name `Remove
  Band` bleibt Tooltip und zugängliche Bezeichnung; Undo stellt das Band
  wieder her (Block 02). Als Expertenweg löscht zusätzlich die Taste
  `Delete` den per Tastatur fokussierten Bandpunkt (Kirchhoff: Löschen per
  Modifikator am Punkt; W3C-Tabs: `Delete` als optionale Schließtaste). Der
  sichtbare Knopf bleibt der Hauptweg (Block 02, 5/5).

## 3/5 — Probeeq: feste Fensterhöhe, zweite Zeile trägt bei EQ aus die Messwahrheit

**Belegte Muster.** Pro-Q 4 ändert seine Fenstergröße ausschließlich auf
Benutzeraktion („Resize button“, VST3-Ziehen der Kanten); „The plugin does
not automatically modify its window height based on state changes.“
Kirchhoff-EQ führt „default window width / default window height / always
use default window size in new instance“ als Einstellungen, ebenfalls
benutzerbestimmt. Im JUCE-Forum sind Größenänderungen des Editors unter FL
Studio wiederholt als unzuverlässig belegt („Bug with calling Editor
setSize() in VST3“, „Plugin GUI size resets after opening VST wrapper
settings“). Nielsen, Heuristik 1: Systemstatus sichtbar halten.

**Entscheidung für Probeeq:**

- Die Kachel hat **eine feste Höhe** (in der Skizze 580×92). Sie ändert
  ihre Fenstergröße nie von selbst; das Tasten-Gesetz gilt sinngemäß für
  das Fenster.
- Zeile 2 ist immer vorhanden. Bei zugeschaltetem EQ trägt sie die laufende
  Änderung und den Modus (Entscheid 5/5 in Block 03). Bei ausgeschaltetem
  EQ trägt sie die Messwahrheit der Sonde: `PASSIVE MEASUREMENT`, Signal-
  und Frischestatus und den Messpunkt (`MODE · POST`). Damit ist keine
  Fläche tot, und der Satz „Zeile 2 nur bei EQ an“ gilt für die EQ-Werte,
  nicht für die Zeile.
- Der offene Punkt „Fensterhöhe“ aus Blueprint Abschnitt 15 Punkt 19 ist
  damit geschlossen.

## 4/5 — Probeeq bei Link-Verlust: stehender Indikator, lokale Wahrheit, kein Dialog

**Belegte Muster.** NN/g unterscheidet Indikatoren („contextual … shown in
close proximity to that element“, passiv), Validierungen und
Benachrichtigungen; ein kritischer Zustand gehört nicht in einen
wegklickbaren Toast, ein Routinezustand nicht in einen Dialog. Die
NN/g-Fehlerregeln verlangen „Display the error message close to the error's
source“, „redundant, and accessible indicators“ (nicht nur Farbe), „Offer
constructive advice“ und „Preserve the user's input“. Der Entwurf (Abschnitt
21) und Blueprint Abschnitt 4.3 halten fest, dass die bestätigte lokale
Klangwahrheit bei Trennung stehen bleibt.

**Entscheidung für Probeeq:**

- Link-Verlust ist ein **stehender Indikator in Zeile 1** am Ort der
  Verbindung: Punkt und Wort (`BROKER OFFLINE`), nicht nur Farbe. Kein
  Dialog, kein Toast, keine Blockade der Kachel.
- Zeile 2 nennt dann die lokale Wahrheit und den nächsten Schritt in einem
  Satz: `LOCAL CONFIRMED · REV n · REMOTE LOCKED · EQ, BYPASS, MIX STAY
  LOCAL`. Der bestätigte Klang läuft weiter; eine laufende Fernänderung
  wird nicht halb angewandt (Entwurf).
- Die lokalen Handgriffe EQ an/aus, Bypass und Mix bleiben im Link-Verlust
  bedienbar; sie sind die Notfallaktionen. Weitere Notfallaktionen
  (Neutralize-Rückfall) bleiben nach Blueprint Abschnitt 4.3 an die
  P6/P7-Recoverytests gebunden und werden hier nicht erfunden.
- Kehrt die Verbindung zurück, verschwindet der Indikator ohne weitere
  Meldung; die Kachel zeigt wieder den Fernzustand.

## 5/5 — Sources-Spalte bei 16 und 32 Quellen: Filterfeld, Scrollliste, feste Zeilen

**Belegte Muster.** Pro-Q 4s Instance List zeigt „all … instances in your
session, organized per track, matching the track order in your DAW“, mit
„a filter text field at the top, which filters the listed tracks as you
type“, einem „Quick Jump“, einem „Filter Pinned“-Knopf und „a minimap … for
sessions with many tracks“. smart:EQ 4 listet Gruppenmitglieder auf der
linken Seite („up to 10 tracks“). Das W3C-Combobox-Muster liefert die
Tastatur des Filterfelds. Nielsen, Heuristik 6: „Minimize the user's memory
load by making elements, actions, and options visible.“

**Entscheidung für Gen Fläche 1:**

- Die Sources-Spalte behält die Mixer-Reihenfolge (Entscheid 23.08.2026)
  und ihre Zeilenhöhe; sie wird bei mehr Quellen eine **Scrollliste**, keine
  dichtere Matrix. Die ausgewählte Quelle bleibt beim Filtern und Scrollen
  ausgewählt und wird beim Öffnen der Fläche in den sichtbaren Bereich
  gerollt.
- Oben in der Spalte sitzt ein **Filterfeld** (`FILTER SOURCES`): Tippen
  filtert die Liste sofort, `Enter` wählt den ersten Treffer, `Escape` leert
  das Feld. Der Kopf nennt die Zahlen ehrlich: `SOURCES 16` beziehungsweise
  beim Filtern `3 MATCH · 16`; angeschlossene Quellen ohne Zeile gibt es
  nicht.
- Quellen ohne Signal oder mit veralteter Messung bleiben in der Liste und
  tragen ihren Zustand in der Zeile; sie werden nicht ausgeblendet
  (Nielsen 1).
- Pinnen und Minimap werden nicht übernommen: bei 32 Quellen reichen Filter
  und Scroll; eine dritte Ordnung wäre Information, die selten gebraucht
  wird (Nielsen 8).

## Quellenledger

- [FabFilter Pro-Q 4 — Loading presets](https://www.fabfilter.com/help/pro-q/presets/loadingpresets), [Saving presets](https://www.fabfilter.com/help/pro-q/presets/savingpresets), [Undo, redo, A/B switch](https://www.fabfilter.com/help/pro-q/using/undoredo), [Solo](https://www.fabfilter.com/help/pro-q/using/solo), [Band controls](https://www.fabfilter.com/help/pro-q/using/bandcontrols), [Instance list](https://www.fabfilter.com/help/pro-q/using/instance-list), [Knobs](https://www.fabfilter.com/help/pro-q/using/knobs), [Display and workflow](https://www.fabfilter.com/help/pro-q/using/eqdisplay), [Full Screen mode, resizing and scaling](https://www.fabfilter.com/help/pro-q/using/fullscreenandresize); alle abgerufen 02.09.2026.
- [Kirchhoff-EQ Manual (PDF)](https://files.plugin-alliance.com/products/tbt_kirchhoff-eq/tbt_kirchhoff-eq_manual.pdf), Abschnitte Undo/Redo, A/B Switch, Preset Selector, Band Operations, Band Control Panel, Settings; abgerufen 02.09.2026, Text per pypdf extrahiert.
- [TDR Nova](https://www.tokyodawn.net/tdr-nova/) und [Nova-Manual](https://docs.tokyodawn.net/nova-manual/): am 02.09.2026 vom Laptop nicht erreichbar (Verbindung verweigert); genutzt wurden der Suchauszug (Solo per Ctrl+Shift, Preset-Manager, Undo/Redo, A/B) und das Quellenledger von Fünferblock 02 vom 01.09.2026.
- [sonible smart:EQ 4](https://www.sonible.com/smarteq4/): Gruppenmitglieder links, bis zu zehn Spuren, Fernsteuerung aus jeder Instanz.
- [iZotope — Inter-plugin communication explained](https://www.izotope.com/en/learn/inter-plugin-communication-explained.html): Instanzen als Knoten, Auswahl per Dropdown; zu Verbindungsverlust nichts dokumentiert.
- [W3C APG — Tabs](https://www.w3.org/WAI/ARIA/apg/patterns/tabs/), [Disclosure](https://www.w3.org/WAI/ARIA/apg/patterns/disclosure/), [Menu Button](https://www.w3.org/WAI/ARIA/apg/patterns/menu-button/), [Combobox](https://www.w3.org/WAI/ARIA/apg/patterns/combobox/).
- [NN/g — 10 Usability Heuristics](https://www.nngroup.com/articles/ten-usability-heuristics/), [Error-Message Guidelines](https://www.nngroup.com/articles/error-message-guidelines/), [Indicators, Validations, and Notifications](https://www.nngroup.com/articles/indicators-validations-notifications/), [Icon Usability](https://www.nngroup.com/articles/icon-usability/).
- JUCE-Forum, Suchauszug 02.09.2026: „Bug with calling Editor setSize() in VST3“, „VST3 plugin editor resizing glitch/issues“, Image-Line-Forum „Plugin GUI size resets after opening VST wrapper settings“.

Der gemeinsame Umbau mit Block 05 und der Sichtbeleg stehen in
[`2026-09-02-technische-ui-architektur-fuenferblock-05.md`](2026-09-02-technische-ui-architektur-fuenferblock-05.md).
