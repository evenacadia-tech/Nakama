# 2026-08-31 — Arbeitsmodus und Stand der technischen UI-Architektur

## Bindender Arbeitsmodus

Der User legt für die neue, designneutrale UI-Ableitung fest:

> „wir machen es so: du stellst mir für jede oberfläche eine frage nach der
> anderen solange bis ich stop sage und ich mir sicher bin, dass wir die selbe
> designsprache sprechen was das grundlegende angeht. es geht hier nochnicht um
> farben und so weiter, sondern grundlegende architektur der ui elemente, was
> sivhtbar ist, was popouts benötigt und all das“

Damit gilt für diese Arbeitsphase:

1. Die Oberflächen werden nacheinander behandelt: Gen Fläche 1, Gen Fläche 2,
   Probeeq und anschließend gemeinsame Regeln.
2. Es wird immer genau **eine** noch offene Architekturfrage gestellt. Die
   Schleife läuft weiter, bis der User sie stoppt oder die grundlegende
   gemeinsame Sprache ausdrücklich bestätigt.
3. Gefragt wird nach Grundaufbau, Hierarchie, sichtbaren Elementen,
   Objektbesitz, Disclosure, Popouts und stabilen Bedienorten. Farben,
   Materialien und visuelle Feinheiten sind in dieser Phase kein Thema.
4. Bereits beantwortete Punkte werden nicht erneut geöffnet.

## Technik ist Vorbedingung, keine Userfrage

Als eine bereits technisch entschiedene Funktionsgrenze erneut als
Auswahlfrage gestellt wurde, korrigierte der User:

> „diese frage ist doch auf der technischen seite fest beschlossen. die frage
> musst du dir selbst erarbeiten und in zukunft keine fragen die mit der technik
> driften.“

Vor jeder weiteren Frage werden deshalb aktueller Code, Tests, Verträge und
technische Fachquellen für den betroffenen Weg geprüft. Funktionsumfang,
Zustandsautorität, technische Grenzen und bereits geschlossene Verträge leitet
Codex selbst daraus ab. Dem User werden nur echte, dadurch nicht entschiedene
UI-Architekturfragen vorgelegt. Ein technischer Konflikt wird benannt und
belegt, nicht als Geschmackswahl getarnt.

## Die Skizze ist das laufende sichtbare Protokoll

Der User verlangt nach den ersten Antworten ausdrücklich:

> „du solltest das aktuelle ui sketch anpassen mit unseren beschlüssen, sonst
> kann ich nicht sehen wo wir stehen“

Nach jeder bestätigten Antwort wird daher zuerst die aktuelle technische
Skizze angepasst und sichtbar geprüft. Erst dann folgt die nächste Frage. Die
Skizze zeigt den laufenden Stand, ist aber nicht alleinige Produktwahrheit:
User-Wortlaut und bindende Konsequenz werden zusätzlich unter `design/abnahmen/`
festgehalten; der Blueprint übernimmt die daraus entstandene aktuelle
Funktionsarchitektur.

### Nachtrag 02.09.2026 — die Skizze ist ein Stand, kein Ergebnis

Der User stellt klar:

> „mir ist einfach wichtig dass verstanden wird : Design ist genauso ein
> lebendiger , sich entwickelnder prozess , wie der laufende Prozess der
> Implementierung des Planes gerade. die Sketch HTML ist der derzeitige
> Stand , nicht der finale . mir ist wichtig dass das alles commited ist"

Bindende Konsequenz:

1. Die technische Skizze liegt ab dem 02.09.2026 ausschließlich im Repo:
   [`design/skizze/nakama-ui-technical-sketch.html`](../skizze/nakama-ui-technical-sketch.html)
   mit Leseblatt [`design/skizze/LIES-MICH.md`](../skizze/LIES-MICH.md) und
   datierten Sichtbelegen unter `design/skizze/belege/`. Der bis dahin
   genutzte Codex-Visualisierungsordner im Home-Verzeichnis des PCs ist kein
   Ablageort mehr; jede Fortschreibung nach einem Fünferblock wird dort im
   Repo committet.
2. Die Skizze ist der derzeitige Stand der entschiedenen Funktionsarchitektur
   und wird nirgends als final, fertig oder abgenommen bezeichnet. Bindend
   bleiben allein die datierten Abnahmen; die Skizze macht sie sichtbar.
3. Zeigt die Skizze etwas, das keine Abnahme deckt, wird das im Leseblatt als
   Vorgriff oder Lücke benannt und in einem folgenden Fünferblock gefragt,
   nicht still entschieden.

## Taktänderung am 2026-09-01 — feste Fünferblöcke

Der User ändert den Umbautakt ausdrücklich:

> „WÄRE ES nicht sinnvoller wir sammeln wieder ein paar entscheidungen? nach
> jeder frage direkt umbauen ist doch sehr umständlich ? wir können das fest
> machen. immer 5 fragen dann umbauen“

Dieser neuere Entscheid ersetzt die Einzelumbau-Regel für den weiteren Takt.
Die übrigen Grenzen dieses Arbeitsmodus bleiben bestehen:

1. Es wird weiterhin immer genau eine echte, technisch vorbereitete
   Architekturfrage gestellt.
2. Fünf beantwortete Architekturfragen bilden einen festen Entscheidungsblock.
   Zwischen den Fragen wird nur der Wortlaut im laufenden Blockprotokoll
   gesichert; Skizze, Detailabnahmen und Blueprint werden nicht einzeln
   umgebaut.
3. Nach der fünften Antwort werden alle fünf Entscheide gemeinsam in die
   laufende Skizze und die betroffenen Abnahmen übernommen, im internen Browser
   sichtbar geprüft und anschließend in den Blueprint integriert. Erst danach
   beginnt der nächste Fünferblock.
4. Ein Block wird nur durch einen neuen ausdrücklichen Userauftrag vorzeitig
   umgebaut.
5. Technische Grenzen werden weiterhin vor jeder Frage selbst ermittelt;
   bereits Entschiedenes bleibt geschlossen. Farben, Materialien und visuelle
   Feinheiten bleiben außerhalb dieser Phase.

Der erste abgeschlossene Block steht in
[`2026-09-01-technische-ui-architektur-fuenferblock-01.md`](2026-09-01-technische-ui-architektur-fuenferblock-01.md).
Der nächste Block erhält sein Protokoll mit der ersten beantworteten Frage;
vorher wird keine leere Entscheidungsdatei angelegt.

## Delegierte Ableitung am 01.09.2026

Für die restlichen Fragen des laufenden zweiten Fünferblocks gibt der User den
zeitlichen Dialog ausdrücklich an eine belegte Ableitung aus etablierten
EQ-Handbüchern ab:

> „bitte schaue dir die manuals von TDR Nova FABFILTER und bekannten eq an und
> leite sinnvolle UX freundliche methoden ab zu den weiteren fragen, habe keine
> zeit mehr“

Codex schließt die drei ausstehenden Fragen dieses Blocks daher ohne weitere
Auswahlaufforderung. Maßgeblich sind offizielle Herstellerhandbücher, der
aktuelle Nakama-Parametervertrag und die bereits bestätigten graphdominanten
Panelregeln. Übertragbar sind wiederkehrende Bedienprinzipien, nicht Farben,
Material, Maße oder die Markenoptik der Referenzprodukte. Die konkrete
Ableitung und ihr Quellenledger stehen in
[`2026-09-01-technische-ui-architektur-fuenferblock-02.md`](2026-09-01-technische-ui-architektur-fuenferblock-02.md).

## Abgenommener Stand — Gen Fläche 1

Die bisher einzeln bestätigten Strukturentscheide ergeben:

- **Permanente Grundteilung:** Eine vertikale Sources-Spalte steht links und
  überspannt die nutzbare Höhe. Normale Quellenwechsel benötigen keinen
  Popout.
- **Sources-Zeile:** Jede Quelle zeigt dauerhaft Identität, Signalstatus,
  Messaktualität und die kompakte Anzahl offener Findings. Eine permanente
  Mini-Diagnostik je Quelle ist verworfen.
- **Eindeutiger Besitzer:** Genau eine Quelle ist Hauptziel der Detailfläche.
  Weitere Quellen erscheinen nur als klar benannte Referenzen; Aktionen
  betreffen ausschließlich das Hauptziel.
- **Rechte Arbeitsfläche:** Frequenzvisualisierung und Findings liegen dort
  waagerecht über die verfügbare Breite und untereinander. Der frühere rechte
  Inspector ist verworfen. Der User beschreibt die Änderung so:

  > „es wäre sinnvoller wenn die visualisierung der frequenzen waagrecht und
  > die findings spalte ebenfalls waagrecht untereinander wären. dann kann der
  > graph besser dargestellt werden. die sources spalte bleibt wie sie ist.“

- **Stabile Geometrie:** Die Findings-Fläche behält dieselbe feste Höhe. Ein
  Finding verändert Größe, Achsen oder Ausschnitt des Frequenzgraphs nicht.
- **Finding-Fokus:** Normal ist genau ein Finding im Fokus. Vor/Zurück und
  `ALL FINDINGS` wechseln den Inhalt innerhalb derselben festen Fläche.
- **Graphreaktion:** Das ausgewählte Finding markiert seinen Frequenzbereich im
  stabilen Gesamtgraph. Ein Zoom darf nur eine bewusste und reversible Aktion
  sein.
- **Findings-Aufbau:** Inhalt und Höraufgabe stehen links; die technisch
  zulässigen Aktionen behalten rechts einen festen, beim Finding-Wechsel
  stabilen Ort.
- **Technische Grenze:** Gen Fläche 1 darf momentary auditionieren und einen
  unbestätigten Draft übergeben. Sie bestätigt keinen EQ und startet keinen
  10-Sekunden-Kandidaten.
- **Einziger Vorwärtsweg:** Im Produkt führt ausschließlich `SEND DRAFT` von
  Gen Fläche 1 zu Gen Fläche 2. Der User entscheidet:

  > „keine funktion um auf die 2. seite zu wechseln abgesehen von send draft“

  Ein Oberflächenumschalter des externen Skizzenwerkzeugs ist kein Element der
  späteren Produktoberfläche.

  **Überholt am 02.09.2026** (Fünferblock 03, Entscheidung 3/5): Der User
  korrigiert „nein hin geht es nicht nur über send draft, sondern über den
  header wo EQ steht“. Der Kopf von Gen trägt den Flächenwechsel; `SEND
  DRAFT` bleibt als Handgriff am Befund, ist aber nicht mehr der einzige
  Hinweg. Wortlaut und Konsequenz in
  [`2026-09-02-technische-ui-architektur-fuenferblock-03.md`](2026-09-02-technische-ui-architektur-fuenferblock-03.md).
  Die Inspektionsleiste der Skizze bleibt davon unberührt Werkzeug.

## Abgenommener Stand — Gen Fläche 2

Die Details stehen in
[`2026-08-31-eq-arbeitsflaeche-dominiert.md`](2026-08-31-eq-arbeitsflaeche-dominiert.md).
Zusammengefasst gilt:

- Der EQ-Graph ist die dominante Arbeitsfläche.
- Eine permanente rechte Transaktionsspalte mit großen Aktionen ist
  verworfen. Draft, Audition, Apply und Reject liegen kompakt unter dem Graphen
  und die Zeile verschwindet ohne offenen Draft.
- Eine permanente Band-Parameterzeile ist verworfen.
- Doppelklick auf einen vorhandenen Bandpunkt öffnet ein kompaktes,
  zweidimensionales und objektgebundenes Mini-Panel für Filter Type, Frequency,
  Gain, Q und Dynamic. Es wird am Bandpunkt je nach verfügbarem Raum ober- oder
  unterhalb platziert und an den Graphkanten innerhalb der Arbeitsfläche
  gehalten; eine feste horizontale Schiene ist verworfen.
- Der Filtertyp wird über sein Kurvensymbol im Mini-Panel und eine kompakte
  2×3-Auswahl der sechs typischen Filterkurven gewechselt. Ausgeschriebene
  All-Caps-Typnamen sind dort verworfen; die Namen bleiben als Tooltip und
  zugängliche Control-Bezeichnung erhalten. Der Band-Slot bleibt beim Wechsel
  derselbe; ein neues Band beginnt als `bell`. Solange die Typauswahl offen
  ist, ersetzt sie die Wertezeile im selben Grundkörper statt das Panel zu
  einem weiteren großen Popup zu verlängern.
- Doppelklick auf eine freie Graphposition belegt dort den nächsten freien der
  acht Band-Slots, wählt ihn aus und öffnet dasselbe Mini-Panel. Sind alle acht
  Slots belegt, meldet die Oberfläche das am Graphen und überschreibt nichts.
- Der Dynamic-Entscheid wird in
  [`2026-09-01-dynamic-ansicht-im-band-panel.md`](2026-09-01-dynamic-ansicht-im-band-panel.md)
  fortgeführt: Für ein aktives Dynamic-Band ersetzt eine zweireihige Ansicht
  mit einem kompakten Zustandscontrol sowie `Range`, `Threshold`, `Attack`,
  `Hold` und `Release` die normalen Bandwerte innerhalb desselben verankerten
  Panelkörpers. `DYN · OFF` aktiviert und öffnet; der Zustandscontrol schaltet
  aus und kehrt ohne Werteverlust zu `Frequency`, `Gain` und `Q` zurück.
- Ein Leerklick schließt das Mini-Panel nicht. Der sichtbare Schließen-Control
  und, außerhalb einer laufenden Zahleneingabe, `Escape` schließen es und
  fokussieren den Bandpunkt, ohne Parameter zu ändern.
- Aktives Dynamic bleibt über eine zweite Kontur am Bandpunkt sichtbar. Die
  Sollposition und das Zeigerziel bleiben dort stabil; innerer Punkt und
  zugehöriger Kurvenzug sind für die tatsächliche Live-Auslenkung vorgesehen.
  `Frame.band_dynamic_gain_db` ist dafür als Name reserviert; Feld-ID und
  Runtime-Nutzlast entstehen erst mit S26–28 und dürfen vorher nicht simuliert
  werden.
- Ein geöffnetes Band-Panel bindet sich bei Auswahl eines anderen vorhandenen
  Bands atomar an dieses Band und kehrt in dessen Grundansicht zurück. Der
  bandlokale Kanalmodus bleibt dort als kompakter Zustandscontrol sichtbar und
  öffnet die fünf vertraglichen Modi im selben Panelkörper.
- `enabled` besitzt im Panelkopf einen stabilen `ON/OFF`-Control. Ein
  ausgeschaltetes Band bleibt als eindeutig inaktives, auswählbares Objekt im
  Graphen sichtbar; seine Werte und seine Slot-ID bleiben erhalten.
- `Remove Band` ist eine getrennte, wiederherstellbare Aktion. Nur sie gibt
  einen der acht Slots frei; weder Bypass noch Panel-Schließen tun das. Der
  native Remove-Weg bleibt bis zu einem versionierten Slot-/Undo-Vertrag
  technisch blockiert.

## Abgenommener Stand — Fünferblock 03 (02.09.2026)

Wortlaut und Sichtbeleg in
[`2026-09-02-technische-ui-architektur-fuenferblock-03.md`](2026-09-02-technische-ui-architektur-fuenferblock-03.md).
Zusammengefasst gilt zusätzlich:

- **Gen Fläche 1, Befund-Dichte:** genau drei Zeilen je Befund (Ursache,
  kleinster Test, worauf hören). Sicherheit ist der Zustand des Befunds
  (`READY TO SEND`, `MORE DATA`, `STALE`), der Beleg die markierte Zone im
  Graphen, Alternativen sind weitere Befunde in der Liste. Leitsatz des
  Users: keine sechs Werte je Befund.
- **Beide Gen-Flächen, Flächenwechsel:** Der Kopf trägt beide Flächennamen
  als Umschalter in beide Richtungen; `SEND DRAFT` bleibt Handgriff am
  Befund. Keine automatische Rückkehr nach `APPLY` fest oder `REJECT`; ein
  offener Draft bleibt offen und erscheint am Befund als `DRAFT OPEN · EQ`.
- **Gen Fläche 2, Global-Zeile:** eingeklappt nennt sie nur aktive
  Abweichungen beim Namen; aufgeklappt liegen Input, Output mit Auto-Gain,
  Width, Mono-Bass, Bypass, Mix, A/B, Voreinstellungen und Verlauf.
- **Probeeq:** flache Kachel mit ein bis zwei Zeilen; Zeile 2 nur bei
  zugeschaltetem EQ (Inhalt seit 23.08.2026).
- **Prozess:** Fundament jetzt in der Skizze, das visuelle Design danach in
  Figma (Prozesswort oben).

## Bewusst offen (Stand nach Fünferblock 03)

- Gen Fläche 1: kein offener Architekturpunkt; Karte U21 ist beantwortet.
- Gen Fläche 2: Disclosure von Voreinstellungen und Verlauf innerhalb der
  Global-Sektion; Wortlaut der Halten-Aktion; Kurzform `DEL`; technische
  Vertragsarbeit für Remove/Undo und Live-Dynamic. Bereits entschiedene
  Panelregeln werden nicht erneut geöffnet.
- Probeeq: ob die zweite Zeile die Fensterhöhe ändert oder in fester Höhe
  ein- und ausgeblendet wird (Host-Fenster unter FL); Verhalten bei
  Link-Verlust und lokale Notfallaktionen.
- Die gemeinsamen UI-Regeln wurden noch nicht befragt.

Vor einer weiteren Architekturfrage wird ein neuer Block aus einem noch
offenen Bedienbereich begonnen.

## Vorgezogener Größenentscheid vom 01.09.2026

Der User hat nach Fünferblock 02 ausdrücklich erlaubt, den Größenentscheid
sofort und querschnittlich einzubauen:

> „ja und passe alle relevanten stellen im vertrag und wo es noch nötig ist
> an, Kontext darf nicht driften das ist das wichtigste“

Diese Anweisung ist eine bewusste Ausnahme vom Fünfer-Takt, keine Rückkehr
zum Umbau nach jeder Einzelantwort. Der erste Größenentscheid wurde später am
selben Tag durch den vereinfachenden Userentscheid ersetzt:

> „compact und standard wird zu kompliziert. es ist schon schwer genug ein
> gutes UI layout zu erstellen. es wird eine größe nämlich standard geben.
> compact kann eventuell iwann wenn alles fertig ist angegangen werden.“

Bindend ist
[`2026-09-01-gen-nur-standardgroesse.md`](2026-09-01-gen-nur-standardgroesse.md):
Beide Gen-Flächen verwenden genau eine logische Größe von 950×538. Compact,
Fenster-Resize und eine zweite Layoutfassung sind vertagt; UI-Skalierung
vergrößert nur dieselbe Geometrie. Der nächste reguläre Entscheid beginnt
wieder einen Fünferblock und öffnet diese Größenfrage nicht erneut.

## Prozesswort vom 02.09.2026 — Fundament jetzt, visuelles Design in Figma

Der User ordnet die Phasen in Fünferblock 03 ein:

> „so dass wir später wenn layout /skizze steht daraus auch ein schönes
> design in figma entwickeln können. (wie bei GEN) aktuell machen wir ja das
> fundament danach kommt das visuelle design in figma.“

Die laufende Skizzenphase liefert das Fundament (Layout, Hierarchie,
Handgriffe, Zustände) für beide Gen-Flächen und Probeeq. Die visuelle Phase
folgt danach in Figma; sie beginnt weiterhin erst mit dem ausdrücklichen
Wechsel, und die Figma-Stände bis zum 25.08.2026 bleiben Verlauf. Wortlaut
und Folge in
[`2026-09-02-technische-ui-architektur-fuenferblock-03.md`](2026-09-02-technische-ui-architektur-fuenferblock-03.md).
