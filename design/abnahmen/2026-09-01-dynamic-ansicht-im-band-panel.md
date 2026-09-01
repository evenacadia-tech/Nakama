# 2026-09-01 — Dynamic wechselt im Band-Panel die Ansicht

> **Integrierter Stand:** Der Dynamic-Fünferblock ist abgeschlossen. Der
> vollständige Wortlaut und der gemeinsame Browserbeleg stehen zusätzlich in
> [`2026-09-01-technische-ui-architektur-fuenferblock-01.md`](2026-09-01-technische-ui-architektur-fuenferblock-01.md).

## Entscheid

Auf die Frage, wie die fünf Dynamic-Werte aus dem kompakten Band-Panel
erreichbar werden, antwortet der User:

> „bezüglich dynamic : A“

Gemeint ist die unmittelbar zuvor gezeigte Variante A: Derselbe am Bandpunkt
verankerte Panelkörper wechselt in eine kompakte zweireihige Dynamic-Ansicht.
Damit gilt für Gen Fläche 2:

- Das Band-Panel erhält weder ein angedocktes Nebenpanel noch ein nach unten
  wachsendes Akkordeon.
- Beim Wechsel in Dynamic bleiben Bandidentität, Bandpunkt und Panelanker
  stabil. Die normalen Felder `Frequency`, `Gain` und `Q` werden für diesen
  Teilschritt durch `Range`, `Threshold`, `Attack`, `Hold` und `Release`
  ersetzt; beide Feldsätze stehen nicht gleichzeitig im Graphen.
- Die fünf Dynamic-Werte werden innerhalb desselben Grundkörpers auf zwei
  kompakte Reihen verteilt. Die aktuelle 2+3-Verteilung im Funktionsblatt ist
  eine technische Dichteprüfung, noch keine Festlegung visueller Maße.
- Der Rückweg stellt im selben Panel die normalen Bandwerte wieder her. Das
  Schließen des Dynamic-Teils darf weder das Band noch seinen Dynamic-Zustand
  still verändern.
- `priority_sidechain` erscheint vor P8 weiterhin nicht in dieser Ansicht.

## Ergänzende Entscheide aus Fünferblock 01

Der User entscheidet nacheinander:

> „C“

> „A“

> „A“

> „B“

> „A ; DAS BANDSCHWINGT ja dann eh sichtbar mit, daran erkennt man es auch“

Im Kontext der jeweils einzeln gestellten Architekturfrage folgt daraus:

1. Ein Klick auf `DYN · OFF` aktiviert `dynamic_enabled` und öffnet die
   Dynamic-Ansicht in einem Schritt. `DYN · ON` öffnet oder verlässt nur die
   Ansicht; das Verlassen oder Schließen deaktiviert Dynamic nicht.
2. Am Anfang der Dynamic-Ansicht sitzt ein kompakter, echter
   `dynamic_enabled`-Zustandscontrol. Er ist der ausdrückliche Ausschalter und
   ändert beim Ausschalten ausschließlich `dynamic_enabled`.
3. Ausschalten lässt den Panelkörper offen, stellt `Frequency`, `Gain` und `Q`
   wieder her und setzt den Fokus auf `DYN · OFF`. Range, Threshold, Attack,
   Hold und Release bleiben gespeichert.
4. Per Zeiger schließt nur der sichtbare Schließen-Control des Panels. Ein
   einfacher Klick auf freie Graphfläche verändert weder Panel, Auswahl noch
   Parameter. `Escape` verwirft zuerst eine laufende Zahleneingabe;
   andernfalls schließt es das oberste nichtmodale Panel. Beide Schließwege
   geben den Fokus an den Bandpunkt zurück und verändern keinen Klangzustand.
5. Ein aktives Dynamic-Band bleibt durch eine zweite Kontur am Bandpunkt
   erkennbar. Diese ruhige Kontur markiert Sollposition und stabiles
   Bedienziel; der innere Punkt und der zugehörige Kurvenzug folgen der
   tatsächlichen Gain-Auslenkung. Die Bewegung ergänzt die Kontur, weil ein
   korrekt aktives Band in stillen Passagen oder bei Range 0 auch unbewegt sein
   kann.

## Technische Grenze

Der Parametervertrag besitzt je festem Band-Slot getrennt
`dynamic_enabled`, `dynamic_range_db`, `threshold_db`, `attack_ms`, `hold_ms`
und `release_ms`. `dynamic_enabled` ist ein topologischer Blockrandwechsel;
die fünf Werte sind kontinuierliche Rampenparameter mit eigenen Einheiten und
Grenzen. Das Öffnen einer Detailansicht und eine Klangzustandsänderung sind
deshalb technisch verschiedene Vorgänge.

Der Name `Frame.band_dynamic_gain_db` ist im v3-Register für S26–28
(`SONDE-015`) reserviert. Der heutige Runtime-Vertrag besitzt aber noch keine
Feld-ID oder Nutzlast dafür: Probeeq soll den Wert später mit Anzeigekadenz im
Featureframe liefern, Gens Master-EQ lokal ohne IPC. Die zweite Kontur kann
aus `dynamic_enabled` wahrheitsgemäß dargestellt werden. Die fortlaufende
Bewegung des inneren Punkts benötigt vor der nativen Umsetzung einen
versionierten Telemetrieweg und darf nicht aus Range oder Threshold simuliert
werden. Das Funktionsblatt kennzeichnet sich deshalb als Zielverhalten und
zeigt nur beim Aktivieren eine einmalige Bewegungsprobe, keine scheinbare
laufende Messung.

## Sichtprüfung

Das laufende Funktionsblatt zeigt B3 mit aktiver Dynamic-Ansicht. Nach 5/5
wurden im internen Browser geprüft:

- der kompakte Ein-/Ausschalter plus fünf Werte als sechs Zellen in genau zwei
  Reihen;
- `DYN · OFF` → aktivieren/öffnen, `DYNAMIC · ON` → ausschalten/zurück und
  `DYN · ON` → Ansicht verlassen, jeweils ohne Verlust der fünf Werte;
- Fokus auf `Range` beim Öffnen und auf `DYN · OFF` nach dem Ausschalten;
- explizites Schließen und zweistufiges Escape-Verhalten mit Rückfokus zum
  Bandpunkt;
- wirkungsloser einfacher Leerklick im Graph;
- sichtbare Dynamic-Konturen für B3 und B5 auch bei geschlossener Werteansicht
  sowie die endliche Bewegungsprobe von innerem Punkt und Kurvenzug beim
  Aktivieren;
- der 238 × 114 Pixel große Panelkörper vollständig und kollisionsfrei im
  Graph: unter B3, unter B5 und oberhalb eines nahe am unteren Rand erzeugten
  B7;
- keine Browserfehler oder Warnungen.

Die Prüfung betrifft nur funktionale Geometrie, Zustandswechsel und
Zugänglichkeit. Farben, Material und visuelle Feinheiten sind nicht
Gegenstand dieser Abnahme.
