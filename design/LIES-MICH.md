# design/ — Design der Nakama-Oberflächen

**Seit 22.08.2026 Teil des Nakama-Repos** (User: „ich möchte dass du die
nakama design und nakama arbeitsbereiche zusammenführst … im anschluss wird
dann nurnoch im nakama repo gearbeitet"). Vorher eigenes Repo
`Projekte/Nakama-Design` (Historie per Subtree-Merge erhalten, Commit
`6fd08a1`). Alle Pfade in dieser Datei und in den Unterordnern sind **relativ
zu `design/`**; ältere Texte, die `Nakama-Design/…` sagen, meinen denselben
Ort.

Hier entsteht das Design der beiden Apps als lebende Blätter im Browser. Es
wird KEIN Plugin-Code geschrieben — der lebt in `eq-copilot/` und `broker/`.
Die Produktwahrheit (was die Apps und Flächen sind und dürfen) und das
Register der User-Entscheide stehen in `CLAUDE.md` an der Repo-Wurzel
(Wahrheitskern); hier wird sie nicht abgeschrieben, nur verwiesen.

## Aktuelle Phase: Funktionsneustart vom 31.08.2026

Für die neue funktionale Designableitung gilt der direkte User-Auftrag vom
31.08.2026: Der aktuelle Implementierungsplan wird in einen erweiterbaren
Funktions- und Bedienvertrag übersetzt; bisher entworfene Designs werden dabei
ignoriert. Der neue, designneutrale Einstieg ist
[docs/funktions-und-bedien-blueprint.md](docs/funktions-und-bedien-blueprint.md).
Er übernimmt keine Raster, Farben, Materialien, Icons oder Positionen aus den
unten unter **Verlauf** dokumentierten früheren visuellen Ständen. Diese
dürfen den Blueprint nicht still ergänzen. Eine neue visuelle Vorgabe entsteht
erst in einem eigenen Folgeschritt. Seit der Planprüfung PR2 (02.09.2026)
bindet der Blueprint die Funktions- und Bedienarchitektur beider Apps; der
Entwurf `../docs/FL-Nakama-Sonden-Design-Entwurf.md` bindet Technik,
Verträge, Phasen und Falsifikation; über beiden stehen aktueller Vertrag und
Code (Rangfolge in beiden Köpfen und in `CLAUDE.md`).

## Laufende technische UI-Architektur

Arbeitsmodus, bestätigter Strukturstand und die bewusst offenen nächsten
Entscheide stehen in
[abnahmen/2026-08-31-technische-ui-architektur-arbeitsmodus.md](abnahmen/2026-08-31-technische-ui-architektur-arbeitsmodus.md).
Für diese Phase gilt: genau eine echte UI-Architekturfrage nach der anderen
und technische Grenzen vorher selbst aus aktuellen Quellen ableiten. Die
Antworten werden in festen Fünferblöcken gesammelt; nach der fünften Antwort
werden sichtbare Skizze, Abnahmen und Blueprint gemeinsam aktualisiert und
geprüft. Farben, Material und visuelle Feinheiten bleiben bis zum
ausdrücklichen Wechsel der Phase außen vor.

Stand 02.09.2026: Fünferblock 01
([abnahmen/2026-09-01-technische-ui-architektur-fuenferblock-01.md](abnahmen/2026-09-01-technische-ui-architektur-fuenferblock-01.md),
Dynamic im Band-Panel) und Fünferblock 02
([abnahmen/2026-09-01-technische-ui-architektur-fuenferblock-02.md](abnahmen/2026-09-01-technische-ui-architektur-fuenferblock-02.md),
Bandbesitz, Kanalmodus, Bypass, Remove) sind integriert. Der aktuelle
Größenvertrag für beide Gen-Flächen steht in
[abnahmen/2026-09-01-gen-nur-standardgroesse.md](abnahmen/2026-09-01-gen-nur-standardgroesse.md):
genau eine logische Größe von 950×538. Compact, Fenster-Resize und eine zweite
Layoutfassung sind bis nach Fertigstellung der Standardoberfläche vertagt;
UI-Skalierung vergrößert nur dieselbe Geometrie. Die noch offenen
Bedienbereiche für den nächsten Block stehen in Blueprint Abschnitt 15 und
im Leseblatt der Skizze; das Protokoll eines neuen Blocks entsteht mit der
ersten beantworteten Frage, nie leer vorab.

Die laufende technische Skizze liegt seit dem 02.09.2026 im Repo:
[skizze/nakama-ui-technical-sketch.html](skizze/nakama-ui-technical-sketch.html),
Leseblatt [skizze/LIES-MICH.md](skizze/LIES-MICH.md), Sichtbelege unter
`skizze/belege/`. Sie ist der derzeitige Stand der entschiedenen
Funktionsarchitektur, nicht der finale (User-Wort 02.09.2026: „die Sketch
HTML ist der derzeitige Stand, nicht der finale"), und wird nach jedem
Fünferblock dort fortgeschrieben und committet.

## Aktuelle Quellen, in dieser Reihenfolge

1. **Funktions- und Bedienarchitektur:**
   [docs/funktions-und-bedien-blueprint.md](docs/funktions-und-bedien-blueprint.md).
   Mit **[U]** markierte Teile sind User-entschieden, **[A]** sind
   Layout-Hypothesen, **[O]** offen; Abschnitt 15 führt die offenen
   Entscheidungen und technischen Blocker, Abschnitt 17 den laufenden
   Ableitungsstand.
2. **Bindende Entscheide:** die datierten Dateien unter `abnahmen/`
   ([abnahmen/LIES-MICH.md](abnahmen/LIES-MICH.md) erklärt Abnahme gegen
   Arbeitsannahme). Für die laufende Phase zuerst die Arbeitsmodus-Abnahme
   vom 31.08.2026 und die jüngste Datei
   `technische-ui-architektur-fuenferblock-*.md`.
3. **Sichtbarer Stand:** `skizze/` mit Leseblatt und Belegen; die dort
   benannten Vorgriffe und Lücken sind die Kandidaten der nächsten Fragen.
4. **Offene Fragen an den User:** Karten in `../docs/plan/fragen.json`,
   gestellt mit dem Skill **`/fragen`** — er hält die Antwort wörtlich fest
   und arbeitet sie ein (`../docs/plan/LIES-MICH.md`). Karten mit
   Designbezug werden zusätzlich im laufenden Blockprotokoll gesichert.
5. **Handwerk, keine Vorgabe:**
   [docs/DESIGN-GESETZE.md](docs/DESIGN-GESETZE.md) (Progressive Disclosure,
   Shneiderman-Mantra, Overview+Detail / Focus+Context / Zoom,
   Aufmerksamkeitsgesetze; Prüfmaßstab für Zustände und Grenzfälle, die
   kein Stand zeigt),
   [docs/CLAUDE-UX-UI-ARBEITSKERN.md](docs/CLAUDE-UX-UI-ARBEITSKERN.md)
   (UX vor UI, Stufen 1–7, Schlusscheck) und als Nachschlagewerk
   [docs/UX-UI-WISSENSBASIS-FUER-CLAUDE.md](docs/UX-UI-WISSENSBASIS-FUER-CLAUDE.md).
   Sie liefern Methode, nie Anordnung, Richtung oder Farbwelt.

Den nächsten Produktauftrag bestimmt `../docs/PLAN-STAND.md`, nicht eine
Design-Phasennummer.

## Harte Regeln (nicht verhandelbar)

1. **Aktuelle Phase zuerst.** Für die funktionale UI-Architektur seit
   31.08.2026 sind Blueprint und neue datierte Abnahmen die erste
   Anlaufstelle. `assets/` und die drei Bilder unter
   `assets/rework-basis-2026-08-25/` bleiben unverändert als Verlauf erhalten,
   liefern in dieser Phase aber weder Layout noch Stil. Erst nach einem
   ausdrücklichen Wechsel in die visuelle Phase wird Figma als Werkzeug
   dieser Phase wieder verbindlich (User-Wort 02.09.2026: „aktuell machen wir
   ja das fundament danach kommt das visuelle design in figma“); welche
   Figma-Datei dann gilt, benennt der User beim Wechsel.
2. **Ein Schritt pro Runde, fünf pro Umbau.** Genau eine Frage öffnen und das
   User-Urteil im laufenden Fünferblock wörtlich sichern — dann die nächste.
   Nach der fünften Antwort werden lebendes Blatt, Detailabnahmen und Blueprint
   gemeinsam umgebaut und sichtbar geprüft. Keine Groß-Lieferungen mit
   gleichzeitigen Entscheidungen.
3. **Der User entscheidet als Projektleiter die visuelle Richtung, du
   übersetzt.** Seit dem Funktionsneustart vom 31.08.2026 steht sie im
   Blueprint, in den datierten Abnahmen und in der laufenden technischen
   Skizze (`skizze/`); die drei Rework-Bilder vom 25.08. sind Verlauf
   (Regeln 1 und 7). Varianten sind nur dort nötig, wo diese
   Vorgaben eine konkrete Frage offenlassen (ein Zustand, den kein Stand
   zeigt; ein Grenzfall, der das Bild sprengt). Nichts Grundsätzliches
   (Stilrichtung, Metapher, Farbwelt) ohne sein Wort festlegen oder wieder
   aufmachen. Eine konkrete fehlende oder unklare Vorstellung wird einzeln
   erfragt; der abgeschlossene Anfangsprozess wird dafür nicht neu gestartet.
   Die Briefing-Seite ist seit dem 23.08.2026 abgeschafft.
4. **Abnahmen sind bindend.** Jedes User-Urteil sofort mit Wortlaut im
   laufenden Fünferblock unter `abnahmen/` festhalten (Format dort). Nach 5/5
   werden die betroffenen Detailabnahmen gemeinsam fortgeschrieben.
   Festgelegtes wird nicht ohne neues User-Wort erneut zur Diskussion gestellt
   — genau das erzeugt die „Ausbessern-Zeremonie", die dieses Projekt
   verhindern soll. Ein neuer ausdrücklicher User-Entscheid oder eine neue
   User-Referenz kann den älteren Stand ersetzen. Produktweite Entscheide
   stehen zusätzlich mit Datum + Zitat an der dafür bezeichneten Stelle gemäß
   Wurzel-`CLAUDE.md`.
5. **Der User gestaltet mit, nicht nur per Zuruf.** In der laufenden
   Funktionsphase heißt das: die technische Skizze schaltet alle Flächen und
   Szenarien über ihre Inspektionsleiste durch, und jede Antwort wird dort
   sichtbar geprüft. Lebende Blätter der visuellen Phase (`werkzeug/`,
   `prototyp/`) bekommen zusätzlich ein eingebautes Bedien-Panel mit live
   verstellbaren Look-Parametern (Farben, Typo-Größen, Dichte, Radien,
   Abstände) und vergleichbaren Varianten — erst nach dem ausdrücklichen
   Wechsel in diese Phase.
6. **Ehrliche Blätter.** In 2 Sekunden ablesbar ist der Maßstab. Nie
   einen Zustand zeigen, den die Spezifikation nicht kennt. Demo-Daten
   sind erlaubt, aber realistisch UND sichtbar als Demo gekennzeichnet.
7. **Kein Kontext-Import aus dem Verlauf.** Für die aktuelle
   Funktionsarchitektur zählen der Blueprint und neuere ausdrückliche
   User-Entscheide. Die drei Rework-Bilder, frühere Figma-Stände sowie
   `../docs/archiv/`, `../eq-copilot/design/archive/`, die geparkte
   Prisma-Studie (`../eq-copilot/design/prisma-studie/`) und das
   Material-Kit-Provisorium (`../eq-copilot/design/` Wurzel) sind Verlauf.
   Aus ihnen wird keine Anordnung, Richtung, Metapher oder Farbwelt geholt.
   Der Wahrheitskern in `CLAUDE.md` gilt der Produktwahrheit; die neue
   visuelle Richtung gehört einem späteren ausdrücklichen User-Schritt.
8. **Nur soviel sichtbar wie der aktuelle Handgriff braucht.** Alles
   andere ist einen Schritt entfernt — nicht weg, aber nicht da.
   Beim Übersetzen: jedes Element des Stands muss beantworten, welchen
   Handgriff es gerade bedient; die Aufgabenliste nach Häufigkeit ist dafür
   die Prüfliste. Varianten — wo es sie noch gibt — unterscheiden sich in
   der **Disclosure-Mechanik** (Schritt-für-Schritt, Focus+Context,
   Overview+Detail, semantischer Zoom, nur-auf-Abruf), nicht in der
   Kastenanordnung. Begründung und Grenzen:
   [docs/DESIGN-GESETZE.md](docs/DESIGN-GESETZE.md).
   Merksatz gegen die naheliegende Ausrede: **nach Häufigkeit der Aufgabe
   teilen, nicht nach Können des Nutzers.**

## Werkzeuge

- **Playwright** (Browser-MCP): jedes gezeigte Ergebnis mit Screenshot
  belegen — „ausgeführt und gesehen", nie „sollte so aussehen". Der MCP lädt
  keine `file://`-Adressen: statischen Server im Ordner `design/` starten
  (`skizze/LIES-MICH.md`), Bilder sofort in den Zielordner verschieben,
  danach `git status --short` lesen.
- **/design**, **/dataviz**: für jede Diagramm-/Meter-Gestaltung (Heatmap,
  EQ-Kurve, Pegel) VOR dem Bauen laden — als Handwerk für die Übersetzung,
  nicht als Quelle einer Richtung.
- **/diverge**: nur bei echten Gabelungen, die die aktuelle User-Vorgabe
  offenlässt.
- **/interview**: nur wenn der User ausdrücklich eine neue Konzeptphase
  öffnet oder eine neue Vorstellung von Grund auf erarbeiten will; nicht als
  Pflichtschleife für den laufenden Umbau.
- **/fragen**: für jede Karte aus `../docs/plan/fragen.json`; nie von Hand
  zusammenfassen.
- **Hooks:** Es gibt keinen Design-Primer und kein Design-Gate beim normalen
  Sessionstart. `.claude/settings.json` injiziert den kompakten Nakama-Primer
  nur nach einer Kontextkomprimierung und schützt Git-Befehle mit dem
  `git-riegel.sh`. Designkontext kommt aus diesem Router, der aktuellen
  Abnahme und dem konkreten Auftrag.
- Technische Skizze = eine einzelne HTML-Datei in `skizze/`, ohne externe
  Abhängigkeiten, per Doppelklick lauffähig; für Browser-Werkzeuge ohne
  Dateizugriff über einen statischen Server im Ordner `design/`
  (`skizze/LIES-MICH.md`). Ihre obere Leiste ist Inspektionswerkzeug, kein
  Produkt-UI.
- Prototyp = statisches HTML/CSS/JS in `prototyp/`, ohne Build-Kette,
  per Doppelklick lauffähig. Er ist der historische Web-Prototyp der
  visuellen Stände bis zum 31.08.2026 und wird in der laufenden
  Funktionsphase nicht fortgeschrieben; sein maschinenlesbarer UI-Vertrag
  (`prototyp/tests/contract.test.mjs`) trägt seit dem 01.09.2026 die eine
  Gen-Größe 950×538 (Blueprint Abschnitt 15, Punkt 13).

## Arbeitsweise

- **Die App ist auf Englisch.** User-Wort vom 2026-08-21: „Englisch – mein
  Wort" (`abnahmen/2026-08-21-designvertrag.md`). Jeder Text, der im Produkt
  steht — Beschriftungen, Werte, Einheiten, Zustände, Meldungen — ist
  englisch. Das gilt auch für die Spezimen in `werkzeug/` und für die
  technische Skizze, weil sie das Produkt zeigen. Das Werkzeug-Chrome darum
  herum (Fragen an den User, Inspektionsleiste) bleibt deutsch beziehungsweise
  ist als Werkzeug gekennzeichnet.
- Docs, Abnahmen, Commits und Gespräch auf Deutsch.
- Die aktuellen Apps heißen Gen und Probeeq. Suna sowie Main /
  Active-Probe-Editor / Passive-Kachel sind Legacy- oder Verlaufsbegriffe.
  Overview und EQ-Zentrale/EQ Center benennen die zwei Gen-Flächen nur
  vorläufig und werden nicht als abgenommene Produktnamen behandelt (offene
  Karte in `../docs/plan/fragen.json`).
- Kleine, benannte Commits nach jedem abgenommenen Schritt — immer mit
  explizitem Pathspec, nie `git add -A` (parallele Sessions, zwei Rechner).
  Plandokumente vor dem Commit durch `../tools/plan/dokuriegel.py` ziehen.
- Sessionende: eine Zeile in `abnahmen/` was offen blieb — kein
  Wissenstransfer über Gedächtnis. Am Planstand ist **nichts** nachzuziehen:
  er wird gerechnet (`../docs/plan/LIES-MICH.md`, seit 23.08.2026).

## Verlauf bis zum 31.08.2026 — Spurensuche, keine Vorgabe

Alles in diesem Abschnitt ist Verlauf. Es bleibt unverändert liegen, darf zur
Spurensuche gelesen werden und liefert der laufenden Funktionsarchitektur
weder Raster noch Farben, Material oder Positionen (Regeln 1 und 7).

- **Drei Rework-Bilder vom 25.08.2026.** Seit dem damaligen User-Entscheid
  waren genau drei gelieferte PNGs die visuelle Wahrheit in progress und die
  Grundlage des damals geplanten großen Reworks:
  `assets/rework-basis-2026-08-25/gen-page-1-overview.png`,
  `assets/rework-basis-2026-08-25/gen-page-2-eq-center.png` und
  `assets/rework-basis-2026-08-25/probeeq.png`. Wortlaut, Hashes und
  Leseregel: `abnahmen/2026-08-25-rework-referenz-drei-designs.md`. Der
  Planschritt S31b hat diese Bilder am 02.09.2026 (PR2) als Quelle
  verloren; er baut auf Blueprint plus einer noch abzunehmenden visuellen
  Spezifikation.
- **Figma-Depot `Nakama-Design` (22.08. bis 25.08.2026).**
  (https://www.figma.com/design/NPCQYSkoZEd4Av0NlKxBOd/Nakama-Design?node-id=0-1,
  Key `NPCQYSkoZEd4Av0NlKxBOd`). User-Wort 22.08.: „das ist das neue
  verbindliche und einzige Depot indem sich die aktuellsten Designs
  befinden"; am 24.08. bekräftigt: „die sind IMMER in diesem figma, nirgends
  wo anders. hier ist immer das aktuellste". Helle und dunkle Varianten waren
  geplant, „stand jetzt aber erstmal nur dunkel"
  (`abnahmen/2026-08-24-figma-depot-immer-aktuell.md`). Die Stände liegen in
  `assets/figma/` (`assets/figma/LIES-MICH.md`); die früheren Depot-Nodes
  `25:444`, `28:994` (Gen) und `6:2864` (Suna) sowie alle Exporte sind
  Verlauf. Das Erfolgskriterium „exakt wie Figma" vom 22.08.2026
  (`abnahmen/2026-08-22-erfolgskriterium-exakt-wie-figma.md`) gilt nicht
  mehr; Beweisziel und Vergleichsgröße kommen aus dem aktuellen Auftrag.
- **Produktstruktur-Entscheide, die weiter gelten**, aber hier nur verlinkt
  sind: zwei Apps seit 23.08.2026
  (`abnahmen/2026-08-23-gen-eq-zentrale.md`), Arbeitsnamen und
  Probeeq-Doppelrolle seit 27.08.2026
  (`abnahmen/2026-08-27-arbeitsnamen-und-probeeq-doppelrolle.md`),
  Auto-Gain-Schalter neben Output vom 24.08.2026
  (`abnahmen/2026-08-24-auto-gain-schalter.md`), Tasten als Material vom
  25.08.2026 (`CLAUDE.md`, Wahrheitskern). Sie sind Produkt- und
  Funktionsentscheide, keine visuellen Stände, und bleiben bindend.
- **Frühere Sachquellen der visuellen Stände:**
  [docs/oberflaechen-spezifikation.md](docs/oberflaechen-spezifikation.md)
  (was jede Ansicht zeigen musste, gestaltet aus den Rework-Bildern),
  [docs/interaktions-und-motion-spezifikation.md](docs/interaktions-und-motion-spezifikation.md),
  [docs/ui-spielregeln-eq-zentrale.md](docs/ui-spielregeln-eq-zentrale.md)
  (seit PR2 am 02.09.2026 ausdrücklich kein Prüfmaßstab für S31b mehr),
  [docs/arbeitsplan.md](docs/arbeitsplan.md) (Design-Arbeitsweg, Stand
  26.08.2026), [docs/werkzeugplan.md](docs/werkzeugplan.md) (Vorschlag vom
  21.08.2026), [docs/sondenplan.md](docs/sondenplan.md) (vermisst den
  verworfenen lokalen Voll-Editor) und die lebenden Blätter samt Messbank in
  `werkzeug/` (`werkzeug/LIES-MICH.md`, `werkzeug/PRUEFLISTE.md`). Wo einer
  dieser Texte einem Blueprint-Satz oder einer neueren Abnahme widerspricht,
  gilt der neuere Stand; ein echter Widerspruch zwischen Entwurf und
  Blueprint wird als Zeile in `../docs/offene-punkte.md` geführt.
- **Anfangsprozess.** Die grundlegende Struktur-, Skizzen- und Graybox-Phase
  war der erste Schritt am Projektanfang und ist abgeschlossen; sie ist kein
  wiederkehrendes Gate (`abnahmen/2026-08-25-skizzen-vor-figma.md`, Nachtrag
  26.08.). Der Funktionsneustart vom 31.08.2026 hat die Bedienarchitektur
  neu abgeleitet, nicht diesen Anfangsprozess wiederholt.
