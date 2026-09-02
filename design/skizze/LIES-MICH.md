# design/skizze — die laufende technische UI-Skizze

**Stand: 02.09.2026 nach Fünferblock 03 · derzeitiger Stand, nicht der
finale.**

User-Wort 02.09.2026:

> „Design ist genauso ein lebendiger, sich entwickelnder prozess, wie der
> laufende Prozess der Implementierung des Planes gerade. die Sketch HTML ist
> der derzeitige Stand, nicht der finale."

Daraus folgt für diesen Ordner:

1. `nakama-ui-technical-sketch.html` ist das sichtbare Protokoll der bisher
   entschiedenen UI-Architektur (Arbeitsmodus vom 31.08.2026, Fünferblöcke
   01 bis 03, Größenentscheid 01.09.2026). Sie zeigt Zielverhalten, nicht den
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

`belege/` enthält Bildschirmfotos des Stands nach Fünferblock 03 vom
02.09.2026, gerendert bei Viewport 1500×900 ohne Skalierung (Gen-Bühne
950×538 logisch = 950×538 Pixel):

| Datei | Zustand |
|---|---|
| `2026-09-02-b03-gen-flaeche-1-kopf.png` | Gen Fläche 1: Kopf mit `OVERVIEW` aktiv und `EQ`, Sources-Spalte 180 px, Evidenz 738×312, Findings 738×142 mit drei Zeilen, `READY TO SEND`, `SEND DRAFT → EQ` |
| `2026-09-02-b03-gen-flaeche-1-draft-open.png` | Gen Fläche 1 nach Rückweg über den Kopf: Befund `DRAFT OPEN · EQ`, Handgriff `OPEN IN EQ →` |
| `2026-09-02-b03-gen-flaeche-2-global-zeile.png` | Gen Fläche 2: Kopf `EQ` aktiv, Band-Panel B3 Grundansicht, eingeklappte Global-Zeile `GLOBAL ⌄ MIX 92 % · AUTO`, Transaktionszeile |
| `2026-09-02-b03-gen-flaeche-2-global-offen.png` | Global aufgeklappt: Input, Output mit `AUTO → −0.8 dB`, Auto-Schalter, Width, Mono-Bass, Bypass, Mix, A/B, `PRESET` und `HISTORY` unavailable |
| `2026-09-02-b03-probeeq-kachel-eq-an.png` | Probeeq-Kachel 580×92: Zeile 1 Name, Bus, `LINKED`, `EQ · ON`, `BYPASS · OFF`, Mix; Zeile 2 `REMOTE PROPOSAL READY`, B3-Werte, `MODE · POST` |
| `2026-09-02-b03-probeeq-kachel-eq-aus.png` | Probeeq-Kachel 580×52: nur Zeile 1, `EQ · OFF`, Bypass und Mix nicht bedienbar |

Die Bilder des Vormittagsstands vor Block 03 liegen als Verlauf unter
`belege/2026-09-02-vor-block-03/` (Inspektionsleiste noch mit
Flächenumschalter, Global-Zeile `2 ON`, Probeeq auf 760×430).

Gemessen am 02.09.2026 im Browser (41 Prüfpunkte, PASS, Konsole leer):
Gen-Bühne 950×538; Kopf-Tabs 96×51, Hover ohne Maßänderung; Findings
738×142; `GLOBAL`-Knopf 78×30 bei jeder Nennung gleich; Band-Panel 276×80
(Grundansicht); Probeeq 580×92 beziehungsweise 580×52 ohne Zeilenüberlauf.
Die vollständige Messtabelle steht in
`../abnahmen/2026-09-02-technische-ui-architektur-fuenferblock-03.md`.

## Bekannte Vorgriffe und Lücken gegenüber den Abnahmen

Diese Punkte sind keine Fehler der Skizze, sondern noch nicht befragte oder
noch nicht abgeleitete Bereiche. Sie gehören in die nächsten Fünferblöcke.

- **Voreinstellungen und Verlauf** (Blueprint Abschnitt 15, Punkt 17): Die
  Einstiege `PRESET` und `HISTORY` liegen in der aufgeklappten
  Global-Sektion, sind aber bis zum Entscheid über ihre Disclosure ehrlich
  unavailable; Preset-Liste und Kurven-Kopie (Antwort U18) haben noch keinen
  Ort.
- **Wortlaut der Halten-Aktion** (Punkt 17): Fläche 1 sagt `HOLD TO
  AUDITION` (Abnahme 22.08.2026), Fläche 2 sagt `AUDITION · HOLD`. Ob beide
  Flächen denselben Wortlaut tragen, ist nicht entschieden.
- **Kurzlabel `DEL`** (Punkt 17): Die abgenommene Aktion heißt `Remove
  Band`; die Skizze zeigt im Panelkopf die Kurzform `DEL` und trägt den
  vollen Namen als zugängliche Bezeichnung. Dichteprüfung, kein Beschluss.
- **Flächennamen** `OVERVIEW` und `EQ` im Kopf sind Arbeitsnamen; die
  Produktnamen sind Karte U23 in `docs/plan/fragen.json`. Ebenso sind
  `DRAFT OPEN · EQ` und `OPEN IN EQ →` Dichteprüfung des Wortlauts.
- **Probeeq-Fensterhöhe** (Punkt 19): Die Kachel blendet mit der zweiten
  Zeile die Bühnenhöhe mit (92 zu 52 Pixel). Ob das Host-Fenster unter FL
  die Höhe ändern darf oder die zweite Zeile in fester Höhe ein- und
  ausgeblendet wird, ist nicht entschieden. Verhalten bei Link-Verlust und
  lokale Notfallaktionen der Kachel sind nicht befragt.
- **Auto-Gain-Ausgleich:** Der Wert `−0.8 dB` neben `OUTPUT · AUTO` ist ein
  Demo-Wert; der echte Ausgleich ist eine deterministische Funktion der
  Kurve (Abnahme 24.08.2026) und entsteht in S26–28/S28b.
- **Fortlaufende Dynamic-Bewegung und native Remove-/Undo-Transaktion** sind
  technische Vertragslücken (Blueprint Abschnitt 15, Punkte 15 und 16); die
  Skizze zeigt nur eine endliche Bewegungsprobe und das Zielverhalten.
- **Gemeinsame UI-Regeln** (Fokus, Tastaturwege über beide Flächen, Fehler-
  und Degradationszustände) wurden noch in keiner Fragenrunde befragt.

## Was hier nicht liegt

Farben, Material, Typografie und visuelle Feinheiten. Sie beginnen erst nach
dem ausdrücklichen Wechsel in die visuelle Phase, die in Figma stattfindet
(CLAUDE.md, Design-Arbeitsmodell Punkt 4; Prozesswort 02.09.2026). Die
Farb- und Schriftwerte in der Skizze sind Werkzeugdefaults für Lesbarkeit,
keine Vorgabe.
