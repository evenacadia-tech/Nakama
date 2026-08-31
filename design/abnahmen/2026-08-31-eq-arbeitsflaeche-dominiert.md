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
  bleiben Gegenstand der folgenden Einzelentscheide.

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
