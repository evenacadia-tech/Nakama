# 2026-09-01 — Gen-Fenster wird größer und bleibt kompakt nutzbar

## Anlass und User-Wortlaut

Der User stellt die bisherige feste 760×430-Fläche wegen des knappen
Informationsraums ausdrücklich zur Diskussion:

> „ICH DENKE es würde helfen wenn wir das layout generell vergrößern würden.
> dann haben wir mehr platz. wie machen das andere plugins ?“

Nach dem Vergleich von Größenstufen, freiem Resize und Vollbild bei
FabFilter Pro-Q 4, fester UI-Skalierung bei TDR Nova sowie frei
konfigurierbarer Fensterfläche bei DMG EQuilibrium wurde folgende
Architekturkonsequenz vorgelegt:

> „760×430 kompakt, ungefähr 950×538 als Standard, vergrößerbar bei stabilem
> Seitenverhältnis, wobei zusätzlicher Raum primär dem Graphen zugutekommt?“

Der User bestätigt und verlangt ausdrücklich den vollständigen
Kontextabgleich:

> „ja und passe alle relevanten stellen im vertrag und wo es noch nötig ist
> an, Kontext darf nicht driften das ist das wichtigste“

## Bindender Größenvertrag für Gen

- Beide Gen-Flächen bleiben gleich groß und teilen denselben Editorrahmen.
- 760×430 logische Pixel sind die **kompakte Mindestgröße**, nicht mehr die
  einzige Größe.
- 950×538 logische Pixel sind die **Standardgröße**. Die Höhe ist die
  ganzzahlige Rundung des beibehaltenen Seitenverhältnisses 76:43.
- Der Gen-Editor ist innerhalb des vom Host und verfügbaren Bildschirm
  erlaubten Bereichs größenveränderbar; das Seitenverhältnis bleibt stabil.
  Ein freies Reflow in andere Formate ist damit weiterhin nicht beschlossen.
- Fensterwachstum ist **Informationsflächenwachstum**, kein proportionaler
  Zoom aller Bedienelemente. Auf Gen Fläche 2 erhält der EQ-Graph den
  zusätzlichen Raum zuerst; auf Gen Fläche 1 gilt dies für die akustische
  Evidenz. Stabile Rails und Bedienziele behalten ihre logischen Maße, solange
  nicht die getrennte UI-Skalierung geändert wird.
- Alle Funktionen müssen bereits bei 760×430 erreichbar bleiben. Mehr Platz
  darf die Wege verkürzen oder Evidenz vergrößern, aber keine Funktion nur in
  der Standard- oder einer größeren Größe verfügbar machen.
- Die beschlossenen UI-Skalierungsstufen 100/125/150/200 Prozent bleiben
  getrennt vom Fenster-Resize. Sie vergrößern die gesamte Oberfläche für
  Lesbarkeit und Bedienbarkeit; Resize vergrößert primär den Arbeitsraum.
- Dieser Entscheid betrifft die beiden Gen-Flächen. Er setzt keine neue Größe
  für die kompakte Probeeq-Rückfallfläche fest und entscheidet noch keinen
  Vollbildmodus oder sichtbaren Größenwahlschalter im Produkt.

## Technische Grenze und Umsetzungsbeweis

Der aktuelle Gen-Pfad in `eq-copilot/plugin/src/PluginEditor.cpp` setzt für
die funktionale Hauptfläche noch `setResizable(false, false)` und 760×430.
Der ältere Resizepfad mit Verhältnis 750:520 gehört zur verworfenen
Material-Kit-Front und ist kein Beweis für diesen neuen Gen-Vertrag.

Vor einer nativen Fertigmeldung sind deshalb in FL Studio zu prüfen:

- 760×430 und 950×538 sowie mindestens eine Zwischen-/größere Stufe;
- festes Verhältnis und ehrliches Clamping am verfügbaren Bildschirm;
- unveränderte Funktionszugänglichkeit und Fokusbesitz nach Resize;
- Panelverankerung an allen Graphkanten in beiden Vertragsgrößen;
- Fenster-Reopen, mehrere Instanzen und 100/125/150/200 Prozent UI-Skalierung;
- visuelle Last, Automation und Audiofristen während des Resize.

Die technische Skizze darf die Größenlogik zeigen. Dieser Entscheid erlaubt
noch keine Spiegelung in den nativen Produktcode vor der vorgesehenen
UI-Implementierungsphase.

## Quellenledger

- [FabFilter Pro-Q 4 — Full Screen, Resizing and Scaling](https://www.fabfilter.com/help/pro-q/using/fullscreenandresize): Größenstufen, freies VST3-Resize, getrennte UI-Skalierung und Vollbild.
- [TDR Nova Manual](https://docs.tokyodawn.net/nova-manual/): feste UI-Skalierungsstufen 100/125/150 Prozent als Gegenbeispiel zu zusätzlicher Informationsfläche.
- [DMG EQuilibrium Manual](https://dmgaudio.com/dl/DMGAudio_EQuilibrium_Manual.pdf): freie Fenstergröße und separat konfigurierbare sichtbare Arbeitsbereiche.

Übernommen wird nur der Mechanismus der getrennten Arbeitsflächenvergrößerung
und UI-Skalierung. Farben, Material, Markenoptik und konkrete Fremdlayouts sind
keine Nakama-Vorgabe.

## Prüfung in der laufenden Session

Die fortgeschriebene technische Skizze wurde unter
`http://127.0.0.1:43117/nakama-ui-technical-sketch.html` im internen Browser
neu geladen und sichtbar geprüft. Weil dessen sichtbare Spalte nur 634 Pixel
breit ist, skaliert ausschließlich der äußere Inspektionsrahmen die vollständige
logische Bühne proportional in die Ansicht; innerhalb der Bühne findet kein
Reflow statt.

Gemessene logische Rechtecke:

| Bereich | Compact | Standard | Wirkung |
|---|---:|---:|---|
| Gen-Bühne | 760×430 | 950×538 | beide Vertragsgrößen exakt vorhanden |
| EQ-Graph | 738×262 | 928×370 | +190 Breite und +108 Höhe |
| Dynamic-Panel B3 | 276×112 | 276×112 | Bedienkörper bleibt stabil |
| Sources-Evidenz | 548×204 | 738×312 | +190 Breite und +108 Höhe |
| Sources-Rail | 180 px breit | 180 px breit | stabile Rail |
| Finding-Bereich | 142 px hoch | 142 px hoch | stabiler Handlungsbereich |

Das Band-Panel blieb für B1 und B6 in beiden Größen vollständig innerhalb des
Graphen. Der Browser meldete keine Warnungen oder Fehler. Der Test
`node design/prototyp/tests/contract.test.mjs` bestand mit dem neuen
UI-Vertrag v2. Der Nakama-Vertragscheck
`check-nakama-contract.ps1 -RepoRoot <workspace>` meldete am 01.09.2026 um
17:49 Uhr `ok: true`, Adaptervertrag `0.3.0` und `failedCount: 0`.

Nicht als erledigt gilt weiterhin die native JUCE-/FL-Studio-Umsetzung. Der
laufende funktionale Editor bleibt bis zur vorgesehenen Implementierungsphase
fest auf 760×430; die Browserprüfung ist ein Architekturbeleg, kein
Produktbeleg.
