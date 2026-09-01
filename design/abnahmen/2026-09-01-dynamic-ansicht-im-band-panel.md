# 2026-09-01 — Dynamic wechselt im Band-Panel die Ansicht

> **Aktueller Sammelstand:** Der nachfolgende Einzelstand wird nach 5/5
> batchweise fortgeschrieben. Entscheidung 1/5 zur Aktivierung beim ersten
> Öffnen steht bis dahin in
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

## Technische Grenze

Der Parametervertrag besitzt je festem Band-Slot getrennt
`dynamic_enabled`, `dynamic_range_db`, `threshold_db`, `attack_ms`, `hold_ms`
und `release_ms`. `dynamic_enabled` ist ein topologischer Blockrandwechsel;
die fünf Werte sind kontinuierliche Rampenparameter mit eigenen Einheiten und
Grenzen. Das Öffnen einer Detailansicht und eine Klangzustandsänderung sind
deshalb technisch verschiedene Vorgänge.

Noch nicht entschieden ist, wie das Ein-/Ausschalten von `dynamic_enabled`
gegenüber dem bloßen Öffnen der bereits aktiven Dynamic-Ansicht bedient wird.
Auch die endgültigen Schließ- und Fokusregeln des gesamten Band-Panels bleiben
offen.

## Sichtprüfung

Das laufende Funktionsblatt zeigt B3 mit aktiver Dynamic-Ansicht. Geprüft
wurden:

- fünf benannte Werte in genau zwei Reihen;
- derselbe 238 Pixel breite Panelkörper wie bei den Grundwerten;
- freie Platzierung unter B3 sowie am weiter rechts liegenden B5;
- vollständige Begrenzung innerhalb der Graphfläche ohne Überdeckung des
  jeweiligen Bandpunkts;
- Wechsel Dynamic → Grundwerte → Dynamic mit Fokus auf `Range` beim Öffnen;
- keine Browserfehler oder Warnungen.

Die Prüfung betrifft nur funktionale Geometrie, Zustandswechsel und
Zugänglichkeit. Farben, Material und visuelle Feinheiten sind nicht
Gegenstand dieser Abnahme.
