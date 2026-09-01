# 2026-08-31 — Die EQ-Arbeitsfläche dominiert Gen Seite 2

## Entscheid

Der User weist die erste technische Skizze der zweiten Gen-Fläche zurück:

> „der eq ist das wichtigste, warum sollten 3 große buttons und etwas text ein
> viertel des displays nehmen? das muss grundlegend verändert werden. das
> widerspricht sich mit UX gesetzen und allem"

Bindende Folge für die weitere Strukturableitung:

- Der EQ-Graph ist die dominante Arbeitsfläche und erhält nicht zugunsten einer
  permanenten großen Transaktionsspalte nur einen Restbereich.
- Draft-, Audition-, Apply- und Reject-Handgriffe bleiben erreichbar, ordnen
  sich aber räumlich und visuell dem EQ unter.
- Eine dauerhafte rechte Spalte mit drei großen Aktionsbuttons und erklärendem
  Fließtext ist verworfen.

## Abnahme des überarbeiteten Entwurfs

Der überarbeitete Entwurf zeigt den EQ über nahezu die volle Breite. Die
Transaktion sitzt als kompakte kontextuelle Zeile darunter und verschwindet
ohne offenen Draft. Der User bestätigt diese Grundhierarchie am 31.08.2026 mit:

> „ok“

Damit ist die Disclosure-Grundform angenommen. Detailform, Maße und visuelle
Ausgestaltung werden weiter einzeln entschieden.

## Objektnahe Bandpräzision

Der User verwirft die dauerhaft sichtbare Präzisionsleiste zugunsten eines
objektgebundenen Handgriffs:

> „doppelklick auf spezifischen punkt des graphen öffnet mini panel mit Q Gain
> Freq + Dyn der TDR NOVA EQ macht das elegant vor. der trick ist funktionen
> mit dem workflow zu integrieren und intuitiv zu machen. das ist doppeltes win
> weil UI gespart und smoother workflow“

Bindende Folge für Gen Seite 2:

- Ein Doppelklick auf einen konkreten Punkt im EQ-Graphen öffnet dort ein
  kompaktes Präzisionspanel für den betroffenen Bandkontext.
- Das Mini-Panel enthält Q, Gain, Frequency und Dynamic; es ist kein
  permanenter Inspector und nimmt dem Graphen im Ruhezustand keine eigene
  Zeile weg.
- Der Graph bleibt der direkte Arbeitsort. Objektwahl, Präzision und sichtbare
  Klangfolge bleiben räumlich zusammen.
- TDR Nova ist Referenz für die workflow-integrierte Disclosure-Mechanik,
  nicht für Nakamas Stil oder eine ungeprüfte 1:1-Gestenbelegung.

## Doppelklick auf eine freie Graphstelle

Auf die Auswahl, ob die Geste auf einer freien Graphstelle ein neues Band
anlegt oder nur für bestehende Bandpunkte gilt, antwortet der User:

> „a“

Damit belegt ein Doppelklick auf eine freie Position im EQ-Graphen den nächsten
freien der acht technischen Band-Slots an der angeklickten Frequenz- und
Gain-Position. Das neue Band ist unmittelbar ausgewählt und sein Mini-Panel
für Q, Gain, Frequency und Dynamic geöffnet. Erzeugen, räumlich platzieren und
präzise weiterarbeiten bilden damit einen einzigen Handgriff.

Sind alle acht Slots belegt, meldet die Oberfläche diesen Zustand am Graphen
ehrlich. Sie überschreibt kein bestehendes Band still. Exakte Schließgeste und
die Detailtiefe hinter `Dyn` bleiben offen.

## Filtertyp im frei verankerten Band-Panel

Für die Frage, ob der Filtertyp im objektgebundenen Mini-Panel gewählt wird,
entscheidet der User Variante A und präzisiert zugleich die Geometrie:

> „A ja .  das popup fenster braucht aber noch arbeit, viel zu groß , es muss
> frei nach oben und unten erscheinen können, jenachdem wo sich der klick
> befindet. aktuell ist es auf schienen und ein langer waagrechter balken. dass
> muss eleganter gehen“

Damit gilt für Gen Seite 2:

- Der Filtertyp gehört als sichtbarer, bandlokaler Handgriff in das Mini-Panel.
  Seine Aktivierung öffnet eine kompakte 2×3-Auswahl der sechs vertraglichen
  Typen `bell`, `low_shelf`, `high_shelf`, `notch`, `low_cut` und `high_cut`.
  Die Typmatrix ersetzt für diesen kurzen Teilschritt die drei Wertefelder im
  selben Panel; nach der Auswahl kehren die Werte zurück.
- Das Mini-Panel ist kompakt zweidimensional aufgebaut. Die bisherige lange
  horizontale Parameterleiste und ihre festen horizontalen Positionen sind
  verworfen.
- Das Panel verankert sich am ausgewählten oder neu angelegten Bandpunkt. Je
  nach verfügbarem Raum erscheint es ober- oder unterhalb des Punkts; an der
  linken und rechten Graphkante bleibt es innerhalb der Arbeitsfläche.
- Ein Typwechsel verändert den Typ desselben festen Band-Slots diskret. Er ist
  kein kontinuierliches Filter-Morphing. Neu angelegte Bänder beginnen gemäß
  Parametervertrag als `bell`.
- Dynamic-Detail sowie endgültige Schließ- und Fokusregeln des Mini-Panels
  bleiben Gegenstand der folgenden Einzelentscheide. Der Dynamic-Ansichtswechsel
  wurde am 01.09.2026 in
  [`2026-09-01-dynamic-ansicht-im-band-panel.md`](2026-09-01-dynamic-ansicht-im-band-panel.md)
  fortgeführt.

## Filterkurven statt All-Caps-Typnamen

Zur sichtbaren Darstellung der sechs Typen präzisiert der User:

> „jeder filter hat typische symbolik die platzsparendes ist als plump die
> wörter hinzuschreiben in CAPS“

Damit gilt:

- Der aktuelle Filtertyp und alle sechs Auswahlziele werden sichtbar durch
  ihre typischen Frequenzgang-Kurven dargestellt: Bell-Wölbung, Shelf-Stufen,
  Notch-Senke sowie ansteigende oder fallende Cut-Kurven.
- Die ausgeschriebenen Typnamen entfallen aus der sichtbaren Auswahl. Sie
  bleiben auf jedem Symbol als englischer Tooltip und zugänglicher Name
  erhalten, damit die platzsparende Expertenabkürzung nicht zum unbenannten
  Control wird.
- Auswahl und aktueller Zustand werden zusätzlich über den Buttonzustand
  vermittelt; die Kurvenform bleibt der Bedeutungsträger des Filtertyps.

## Fortsetzung am 01.09.2026 — Dynamic im selben Panelkörper

Der User entscheidet zur Erreichbarkeit von `Range`, `Threshold`, `Attack`,
`Hold` und `Release`:

> „bezüglich dynamic : A“

Damit wechselt derselbe am Bandpunkt verankerte Panelkörper in eine kompakte
zweireihige Dynamic-Ansicht. Die fünf Werte ersetzen dort vorübergehend
`Frequency`, `Gain` und `Q`; es entsteht weder ein angedocktes Nebenpanel noch
ein nach unten wachsendes Akkordeon. Die genaue Konsequenz und technische
Abgrenzung sind in
[`2026-09-01-dynamic-ansicht-im-band-panel.md`](2026-09-01-dynamic-ansicht-im-band-panel.md)
festgehalten.

## Integrierter Dynamic-Fünferblock vom 01.09.2026

Der abgeschlossene
[`2026-09-01-technische-ui-architektur-fuenferblock-01.md`](2026-09-01-technische-ui-architektur-fuenferblock-01.md)
schließt Aktivierung, Ausschalten, Rückweg, Schließen und den sichtbaren
Aktivbeleg für Dynamic:

- `DYN · OFF` aktiviert und öffnet. `DYN · ON` ist danach nur noch der
  Disclosure-Einstieg; Öffnen, Verlassen und Schließen deaktivieren das Band
  nicht.
- Der echte Ausschalter steht kompakt am Anfang der Dynamic-Ansicht.
  Ausschalten erhält alle fünf Werte, stellt im selben Panel `Frequency`,
  `Gain` und `Q` wieder her und fokussiert `DYN · OFF`.
- Ein einfacher Leerklick im Graph schließt nichts. Per Zeiger schließt nur der
  sichtbare Panel-Control. `Escape` verwirft zuerst eine laufende
  Zahleneingabe und schließt andernfalls das Panel. Schließen verändert keine
  Parameter und fokussiert den Bandpunkt.
- Ein aktives Dynamic-Band besitzt eine zweite, ruhige Kontur an seiner
  eingestellten Position. Der innere Punkt und der zugehörige Kurvenzug folgen
  später der tatsächlichen Gain-Auslenkung; die Kontur bleibt als stabiler
  Zustand und Bedienort stehen.

`Frame.band_dynamic_gain_db` ist für S26–28 als künftiger Name reserviert; der
aktuelle Runtime-Vertrag liefert noch keine Feld-ID oder Nutzlast. Probeeq
führt den Wert später über den Featureframe zu Gen, Gens Master-EQ lokal ohne
IPC. Bis dieser Weg gebaut ist, bleibt die laufende Bewegung eine benannte
technische Lücke und darf nicht aus Einstellwerten erfunden werden. Im
Funktionsblatt ist nur eine endliche Bewegungsprobe des Zielverhaltens
enthalten.

## Integrierter Bandkontext-Fünferblock vom 01.09.2026

Der abgeschlossene
[`2026-09-01-technische-ui-architektur-fuenferblock-02.md`](2026-09-01-technische-ui-architektur-fuenferblock-02.md)
ergänzt das objektgebundene Mini-Panel um die verbleibenden bandlokalen
Grundhandlungen:

- Ein einmal ausgewählter anderer Bandpunkt übernimmt das bereits geöffnete
  Panel atomar. Das Panel verankert sich am neuen Punkt, zeigt dessen
  Grundansicht und schreibt ausschließlich auf diesen Slot.
- Der aktuelle Kanalmodus bleibt als kompakter Control in der Grundansicht
  sichtbar. Seine Aktivierung ersetzt die drei Wertefelder im selben
  Panelkörper durch `Stereo`, `Left`, `Right`, `Mid` und `Side`; danach kehrt
  die Grundansicht zurück.
- `enabled` ist ein eigener, in allen Panelansichten stabiler `ON/OFF`-Control.
  Ein ausgeschaltetes Band bleibt ohne reine Farbcodierung sichtbar,
  auswählbar und mit unveränderten Werten demselben Slot zugeordnet.
- Entfernen ist von Bypass, Dynamic und Schließen getrennt. Nur `Remove Band`
  gibt einen der acht Plätze frei, schließt das Panel und entfernt den Punkt;
  Undo stellt das vollständige Band mit derselben ID wieder her.

Diese Regeln übertragen die wiederkehrende Trennung von Auswahl, Bypass,
Kanalplatzierung und Delete aus den offiziellen Handbüchern von TDR Nova,
FabFilter Pro-Q 4 und Kirchhoff-EQ. Sie übernehmen keine visuelle Sprache der
Referenzprodukte. Der aktuelle Parametervertrag besitzt noch keinen
persistenten Occupancy-/Remove-Zustand; daher ist die native Remove-Transaktion
bis zu einem versionierten Slot- und Undo-Vertrag eine technische Lücke.

## Größenfortschreibung vom 01.09.2026

Die Dominanz der EQ-Arbeitsfläche gilt auf der einzigen logischen Gen-Größe
950×538. Gemäß
[`2026-09-01-gen-nur-standardgroesse.md`](2026-09-01-gen-nur-standardgroesse.md)
werden weder eine Compact-Fassung noch größenabhängiges Reflow parallel
entworfen. Bandziele, Parameterfelder und Rails besitzen deshalb genau eine zu
prüfende Geometrie. Das objektgebundene Panel bleibt am Bandpunkt verankert und
muss an allen Graphkanten dieser Standardfläche vollständig erreichbar sein.
