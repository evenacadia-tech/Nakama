# design/skizze — die laufende technische UI-Skizze

**Stand: 02.09.2026 nach Fünferblock 05 · derzeitiger Stand, nicht der
finale.**

User-Wort 02.09.2026:

> „Design ist genauso ein lebendiger, sich entwickelnder prozess, wie der
> laufende Prozess der Implementierung des Planes gerade. die Sketch HTML ist
> der derzeitige Stand, nicht der finale."

Daraus folgt für diesen Ordner:

1. `nakama-ui-technical-sketch.html` ist das sichtbare Protokoll der bisher
   entschiedenen UI-Architektur (Arbeitsmodus vom 31.08.2026, Fünferblöcke
   01 bis 05, Größenentscheid 01.09.2026). Sie zeigt Zielverhalten, nicht den
   gebauten Editor, und keine visuelle Richtung; ihre eigene Legende sagt
   `TARGET BEHAVIOR · NOT CURRENT BUILD · VISUAL STYLE OPEN`.
2. Sie wird mit jedem abgeschlossenen Fünferblock hier fortgeschrieben und
   committet. Der einzige Ablageort ist dieser Ordner im Repo. Bis zum
   01.09.2026 lag die Datei nur im Codex-Visualisierungsordner des PCs
   (`~/.codex/visualizations/2026/08/31/01a056ed-…/`); am 02.09.2026 wurde sie
   byteidentisch übernommen und danach im Repo weiterentwickelt.
3. Nichts in dieser Datei ist eine Abnahme. Bindend sind ausschließlich die
   datierten Dateien unter `../abnahmen/`; der Blueprint
   `../docs/funktions-und-bedien-blueprint.md` führt die daraus entstandene
   Funktionsarchitektur. Zeigt die Skizze etwas, das keine Abnahme deckt, ist
   das ein Vorgriff oder eine Lücke und wird unten benannt, nicht still
   entschieden.
4. Die Skizze liefert das Fundament (Layout, Hierarchie, Handgriffe,
   Zustände). Das visuelle Design entsteht danach in Figma (User-Wort
   02.09.2026, Prozesswort in Fünferblock 03).

## Öffnen

- Doppelklick auf `nakama-ui-technical-sketch.html`; die Datei ist eine
  einzelne HTML-Datei ohne externe Abhängigkeiten.
- Für Browser-Werkzeuge, die keine Dateipfade laden dürfen: im Ordner `design/`
  einen statischen Server starten und `/skizze/nakama-ui-technical-sketch.html`
  öffnen, zum Beispiel `py -3.13 -m http.server 43118 --bind 127.0.0.1`.
- Die obere Leiste ist Inspektionswerkzeug, kein Produkt-UI: `WINDOW` wählt
  das Plugin-Fenster (`GEN` oder `PROBEEQ`), `SCENARIO` spielt Zustände
  durch (bestätigt, Vorschlag, Kandidat, Link-Verlust). Der Wechsel zwischen
  Gens beiden Flächen ist dagegen Produkt-UI und liegt im Fensterkopf
  (`OVERVIEW` / `EQ`, Entscheid 02.09.2026). Der äußere Rahmen darf die
  Bühne proportional verkleinern; das ist kein Produkt-Resize.
- Automatische Prüfung: Playwright für Python mit dem System-Chrome
  (`py -3.13 -m pip install playwright`, Start mit `channel="chrome"`, kein
  Browser-Download). Das Prüfskript des jeweiligen Blocks liegt im
  Sessionordner und wird im Blockprotokoll unter `../abnahmen/` mit seinen
  Messwerten dokumentiert.

## Sichtbelege

`belege/` enthält Bildschirmfotos vom 02.09.2026, gerendert bei Viewport
1500×900 ohne Skalierung (Gen-Bühne 950×538 logisch = 950×538 Pixel).
Präfix `b03` = Stand nach Fünferblock 03, Präfix `b0405` = Stand nach den
delegierten Blöcken 04 und 05 (aktuell):

| Datei | Zustand |
|---|---|
| `2026-09-02-b0405-gen-flaeche-1-sources-filter.png` | Gen Fläche 1: Sources-Spalte mit Filterfeld `gui`, Kopf `2 MATCH · 16`, Scrollliste; Kopf-Umschalter `OVERVIEW` / `EQ` |
| `2026-09-02-b0405-gen-flaeche-2-preset-liste.png` | Gen Fläche 2: Global aufgeklappt, Preset-Liste im selben Körper (Filter, FACTORY/USER, `SAVE`, `SAVE AS`), `UNDO`/`REDO` in der unteren Zeile, `COPY →` an der Zielleiste |
| `2026-09-02-b0405-gen-flaeche-2-copy-bestaetigung.png` | Zielleiste als Kopie-Zielwahl: `COPY BASS CURVE TO`, Ziel `PIANO` gedrückt, `REPLACES THE PIANO CURVE`, `CONFIRM COPY`, `CANCEL` |
| `2026-09-02-b0405-gen-flaeche-2-global-validierung.png` | Global aufgeklappt mit `PRESET USER 04 *` (geändert), `REVISION`, Validierungszeile `WIDTH · NOT A NUMBER, KEPT 100 %` |
| `2026-09-02-b0405-probeeq-eq-aus-messwahrheit.png` | Probeeq-Kachel 600×92 bei EQ aus: Zeile 2 `EQ OFF · PASSIVE MEASUREMENT · SIGNAL · FRESH 1.2 s`, Bypass und Mix nicht bedienbar |
| `2026-09-02-b0405-probeeq-link-verlust.png` | Probeeq bei Link-Verlust: `BROKER OFFLINE` in Zeile 1, `LOCAL CONFIRMED · REV 128 · REMOTE LOCKED · EQ, BYPASS, MIX STAY LOCAL` in Zeile 2, lokale Handgriffe bedienbar |
| `2026-09-02-b03-gen-flaeche-1-kopf.png` | Block 03: Kopf mit `OVERVIEW` aktiv, Findings 738×142 mit drei Zeilen, `SEND DRAFT → EQ` |
| `2026-09-02-b03-gen-flaeche-1-draft-open.png` | Block 03: nach Rückweg über den Kopf `DRAFT OPEN · EQ`, `OPEN IN EQ →` |
| `2026-09-02-b03-gen-flaeche-2-global-zeile.png` | Block 03: eingeklappte Global-Zeile `GLOBAL ⌄ MIX 92 % · AUTO` |
| `2026-09-02-b03-gen-flaeche-2-global-offen.png` | Block 03: Global aufgeklappt mit `AUTO → −0.8 dB` (noch mit `PRESET`/`HISTORY` unavailable, seit Block 04 ersetzt) |
| `2026-09-02-b03-probeeq-kachel-eq-an.png` | Block 03: Probeeq-Kachel 580×92 mit EQ an (seit Block 04 600×92) |
| `2026-09-02-b03-probeeq-kachel-eq-aus.png` | Block 03: Probeeq 580×52 bei EQ aus (seit Block 04 feste Höhe, Zeile 2 mit Messwahrheit) |

Die Bilder des Vormittagsstands vor Block 03 liegen als Verlauf unter
`belege/2026-09-02-vor-block-03/` (Inspektionsleiste noch mit
Flächenumschalter, Global-Zeile `2 ON`, Probeeq auf 760×430).

Gemessen am 02.09.2026 im Browser (Block 04/05: 65 Prüfpunkte, Regression
Block 03: 42 Prüfpunkte, alle PASS, Konsole leer): Gen-Bühne 950×538;
Kopf-Tabs 96×51, Hover ohne Maßänderung; Findings 738×142; Sources-Spalte
180 px mit Scrollliste; `GLOBAL`-Knopf 78×30, `UNDO`/`REDO` je 58×30,
Halten-Knopf 128×30; Band-Panel 276×80 (Grundansicht); Probeeq 600×92 in
jedem Zustand ohne Zeilenüberlauf. Die Messtabellen stehen in
`../abnahmen/2026-09-02-technische-ui-architektur-fuenferblock-03.md` und
`../abnahmen/2026-09-02-technische-ui-architektur-fuenferblock-05.md`.

## Bekannte Vorgriffe und Lücken gegenüber den Abnahmen

Diese Punkte sind keine Fehler der Skizze, sondern noch nicht befragte oder
noch nicht abgeleitete Bereiche. Sie gehören in die nächsten Fünferblöcke.

- **Flächennamen** `OVERVIEW` und `EQ` im Kopf sind Arbeitsnamen; die
  Produktnamen sind Karte U23 in `docs/plan/fragen.json`. Ebenso sind die
  Hinweistexte (`DRAFT OPEN · EQ`, `OPEN IN EQ →`, `COPIED … TO …`,
  `KEPT 100 %`) Dichteprüfung des Wortlauts.
- **Demo-Bestände:** Der Auto-Gain-Ausgleich `−0.8 dB` ist ein Demo-Wert
  (der echte ist eine deterministische Funktion der Kurve, Abnahme
  24.08.2026, S26–28/S28b); die sieben Voreinstellungen, die 16 Quellen und
  die Kopie, die in der Skizze nur Hinweis und Undo-Eintrag ist, sind
  Demo-Daten. Die Ziele teilen sich in der Skizze denselben Bandsatz.
- **Probeeq-Notfallaktionen:** EQ an/aus, Bypass und Mix sind die lokalen
  Handgriffe, auch bei Link-Verlust. Ob weitere nötig sind (Neutralize-
  Rückfall), entscheiden die P6/P7-Recoverytests, keine Bedienfrage.
- **Fortlaufende Dynamic-Bewegung und native Remove-/Undo-Transaktion** sind
  technische Vertragslücken (Blueprint Abschnitt 15, Punkte 15 und 16); die
  Skizze zeigt nur eine endliche Bewegungsprobe und das Zielverhalten. Auch
  die Preset-, Kopie- und Kandidatentransaktionen warten auf ihre Verträge
  (Punkte 12 und 14).
- **Bedienfragen sind seit Block 05 keine offen.** Was die Skizze zeigt,
  deckt eine datierte Abnahme oder die delegierte Ableitung vom 02.09.2026;
  eine neue Frage entsteht erst aus einem neuen User-Entscheid oder einem
  technischen Vertrag.

## Was hier nicht liegt

Farben, Material, Typografie und visuelle Feinheiten. Sie beginnen erst nach
dem ausdrücklichen Wechsel in die visuelle Phase, die in Figma stattfindet
(CLAUDE.md, Design-Arbeitsmodell Punkt 4; Prozesswort 02.09.2026). Die
Farb- und Schriftwerte in der Skizze sind Werkzeugdefaults für Lesbarkeit,
keine Vorgabe.
