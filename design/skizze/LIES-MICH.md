# design/skizze — die laufende technische UI-Skizze

**Stand: 02.09.2026 · derzeitiger Stand, nicht der finale.**

User-Wort 02.09.2026:

> „Design ist genauso ein lebendiger, sich entwickelnder prozess, wie der
> laufende Prozess der Implementierung des Planes gerade. die Sketch HTML ist
> der derzeitige Stand, nicht der finale."

Daraus folgt für diesen Ordner:

1. `nakama-ui-technical-sketch.html` ist das sichtbare Protokoll der bisher
   entschiedenen UI-Architektur (Arbeitsmodus vom 31.08.2026, Fünferblöcke 01
   und 02, Größenentscheid 01.09.2026). Sie zeigt Zielverhalten, nicht den
   gebauten Editor, und keine visuelle Richtung; ihre eigene Legende sagt
   `TARGET BEHAVIOR · NOT CURRENT BUILD · VISUAL STYLE OPEN`.
2. Sie wird mit jedem abgeschlossenen Fünferblock hier fortgeschrieben und
   committet. Der einzige Ablageort ist dieser Ordner im Repo. Bis zum
   01.09.2026 lag die Datei nur im Codex-Visualisierungsordner des PCs
   (`~/.codex/visualizations/2026/08/31/01a056ed-…/`); am 02.09.2026 wurde sie
   byteidentisch übernommen (SHA-256 beginnt mit `64c923bd2ef5e14b`).
3. Nichts in dieser Datei ist eine Abnahme. Bindend sind ausschließlich die
   datierten Dateien unter `../abnahmen/`; der Blueprint
   `../docs/funktions-und-bedien-blueprint.md` führt die daraus entstandene
   Funktionsarchitektur. Zeigt die Skizze etwas, das keine Abnahme deckt, ist
   das ein Vorgriff oder eine Lücke und wird unten benannt, nicht still
   entschieden.

## Öffnen

- Doppelklick auf `nakama-ui-technical-sketch.html`; die Datei ist eine
  einzelne HTML-Datei ohne externe Abhängigkeiten.
- Für Browser-Werkzeuge, die keine Dateipfade laden dürfen: im Ordner `design/`
  einen statischen Server starten und `/skizze/nakama-ui-technical-sketch.html`
  öffnen, zum Beispiel `py -3.13 -m http.server 43118 --bind 127.0.0.1`.
- Die obere Leiste (`SURFACE`, `SCENARIO`) ist Inspektionswerkzeug, kein
  Produkt-UI. Im Produkt führt ausschließlich `SEND DRAFT` von Gen Fläche 1 zu
  Gen Fläche 2 (User-Wort 31.08.2026). Der äußere Rahmen darf die 950×538-Bühne
  proportional verkleinern; das ist kein Produkt-Resize.

## Sichtbelege

`belege/` enthält Bildschirmfotos des Stands vom 02.09.2026, gerendert bei
Viewport 1500×900 ohne Skalierung (Bühne 950×538 logisch = 950×538 Pixel):

| Datei | Zustand |
|---|---|
| `2026-09-02-gen-flaeche-1-sources.png` | Gen Fläche 1: Sources-Spalte 180 px, Evidenz 738×312, Findings 738×142, Aktionen rechts, `SEND DRAFT → EQ` |
| `2026-09-02-gen-flaeche-2-eq.png` | Gen Fläche 2: Graph dominant, Band-Panel B3 in Dynamic-Ansicht (276×112), Global eingeklappt, Transaktionszeile unter dem Graphen |
| `2026-09-02-gen-flaeche-2-filtertyp.png` | Filtertyp-Auswahl 2×3 als Kurvensymbole im selben Panelkörper |
| `2026-09-02-gen-flaeche-2-band2-aus.png` | Panel nach Einfachklick auf B2 umgebunden: `OFF`, Kanalmodus `L`, Grundansicht, ausgeschalteter Bandpunkt sichtbar |
| `2026-09-02-probeeq.png` | Probeeq-Rückfallfläche, unbefragter Vorgriff (siehe unten) |

Gemessen am 02.09.2026 im Browser: Bühne beider Gen-Flächen 950×538;
Band-Panel 276×80 (Grundansicht), 276×108 (Typauswahl), 276×112 (Dynamic),
jeweils vollständig innerhalb des Graphen; Konsole ohne Fehler der Skizze.

## Bekannte Vorgriffe und Lücken gegenüber den Abnahmen

Diese Punkte sind keine Fehler der Skizze, sondern noch nicht befragte oder
noch nicht abgeleitete Bereiche. Sie gehören in die nächsten Fünferblöcke.

- **Global-Sektion auf Fläche 2:** Der eingeklappte Bereich zeigt nur Input,
  Output, Width und A/B. Der Auto-Gain-Schalter neben Output (Abnahme
  24.08.2026), Bypass, Mix, Preset/Copy und Revision/History aus Blueprint
  Abschnitt 4.2 Zone 4 sind dort noch nicht abgeleitet.
- **Probeeq-Ansicht:** Die Fläche folgt Blueprint Abschnitt 4.3, wurde aber in
  keiner Fragenrunde befragt. Ihre Bühne von 760×430 ist eine
  Werkzeugannahme; die Größe der Probeeq-Rückfallfläche ist nicht entschieden
  (Abnahme 01.09.2026 zur Standardgröße gilt nur für Gen).
- **Rückweg von Fläche 2 zu Fläche 1:** im Produkt noch offen; die Skizze
  bietet ihn nur über das Inspektionswerkzeug.
- **Wortlaut der Halten-Aktion:** Fläche 1 sagt `HOLD TO AUDITION`
  (Abnahme 22.08.2026), Fläche 2 sagt `AUDITION · HOLD`. Ob beide Flächen
  denselben Wortlaut tragen, ist nicht entschieden.
- **Kurzlabel `DEL`:** Die abgenommene Aktion heißt `Remove Band`; die Skizze
  zeigt im Panelkopf die Kurzform `DEL` und trägt den vollen Namen als
  zugängliche Bezeichnung. Die sichtbare Kurzform ist eine Dichteprüfung,
  kein Beschluss.
- **Fortlaufende Dynamic-Bewegung und native Remove-/Undo-Transaktion** sind
  technische Vertragslücken (Blueprint Abschnitt 15, Punkte 15 und 16); die
  Skizze zeigt nur eine endliche Bewegungsprobe und das Zielverhalten.

## Was hier nicht liegt

Farben, Material, Typografie und visuelle Feinheiten. Sie beginnen erst nach
dem ausdrücklichen Wechsel in die visuelle Phase (CLAUDE.md,
Design-Arbeitsmodell Punkt 4). Die Farb- und Schriftwerte in der Skizze sind
Werkzeugdefaults für Lesbarkeit, keine Vorgabe.
