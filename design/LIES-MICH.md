# design/ — Design der Nakama-Oberflächen

**Seit 22.08.2026 Teil des Nakama-Repos** (User: „ich möchte dass du die
nakama design und nakama arbeitsbereiche zusammenführst … im anschluss wird
dann nurnoch im nakama repo gearbeitet"). Vorher eigenes Repo
`Projekte/Nakama-Design` (Historie per Subtree-Merge erhalten, Commit
`6fd08a1`). Alle Pfade in dieser Datei und in den Unterordnern sind **relativ
zu `design/`**; ältere Texte, die `Nakama-Design/…` sagen, meinen denselben
Ort.

Hier entsteht das Design als **Web-Prototyp im Browser**, als Übersetzung der
aktuellen visuellen User-Vorgabe. Es wird KEIN Plugin-Code geschrieben — der lebt in
`eq-copilot/` und `broker/`. Die Produktwahrheit (was die Apps und Flächen sind und
dürfen) und das Register der User-Entscheide stehen in `CLAUDE.md` an der
Repo-Wurzel (Wahrheitskern); hier wird sie nicht abgeschrieben, nur verwiesen.

## Funktionsneustart vom 31.08.2026

Für die neue funktionale Designableitung gilt der direkte User-Auftrag vom
31.08.2026: Der aktuelle Implementierungsplan wird in einen erweiterbaren
Funktions- und Bedienvertrag übersetzt; bisher entworfene Designs werden dabei
ignoriert. Der neue, designneutrale Einstieg ist
[docs/funktions-und-bedien-blueprint.md](docs/funktions-und-bedien-blueprint.md).
Er übernimmt keine Raster, Farben, Materialien, Icons oder Positionen aus den
unten dokumentierten früheren visuellen Ständen. Diese bleiben Verlauf und
dürfen den Blueprint nicht still ergänzen. Eine neue visuelle Vorgabe entsteht
erst in einem eigenen Folgeschritt.

## Laufende technische UI-Architektur vom 31.08.2026

Arbeitsmodus, bestätigter Strukturstand und die bewusst offenen nächsten
Entscheide stehen in
[`abnahmen/2026-08-31-technische-ui-architektur-arbeitsmodus.md`](abnahmen/2026-08-31-technische-ui-architektur-arbeitsmodus.md).
Für diese Phase gilt: genau eine echte UI-Architekturfrage nach der anderen
und technische Grenzen vorher selbst aus aktuellen Quellen ableiten. Die
Antworten werden in festen Fünferblöcken gesammelt; nach der fünften Antwort
werden sichtbare Skizze, Abnahmen und Blueprint gemeinsam aktualisiert und
geprüft. Farben, Material und visuelle Feinheiten bleiben bis zum
ausdrücklichen Wechsel der Phase außen vor.

## Dein Job

**Historischer visueller Arbeitsstand bis zum Funktionsneustart vom
31.08.2026:** Seit dem damaligen User-Entscheid vom 25.08. waren genau drei
gelieferte PNGs die **visuelle Wahrheit in progress** und die Grundlage des
damals geplanten großen Reworks:

- `assets/rework-basis-2026-08-25/gen-page-1-overview.png`
- `assets/rework-basis-2026-08-25/gen-page-2-eq-center.png`
- `assets/rework-basis-2026-08-25/probeeq.png`

Wortlaut, Hashes und Leseregel stehen in
`abnahmen/2026-08-25-rework-referenz-drei-designs.md`. Die frühere Regel
„immer ausschließlich der Live-Stand in Nakama-Design“ ist für die aktuelle
visuelle Ausgangsbasis durch diesen neueren Entscheid abgelöst. Figma und
`assets/figma/` sind bis zu einer späteren ausdrücklichen Aktualisierung
Verlauf, nicht stiller Gegenspieler dieser drei Bilder. Dieser Ordner
macht daraus **lebende Blätter** und prüft, was ein Bild nicht prüfen kann:
**Zustände** (frisch / veraltet / getrennt, Messung, degradiert, Draft,
Bypass …), **Größen** (bei 1:1 auf den abgenommenen Maßen) und
**Grenzfälle** (16 Quellen, alles getrennt, acht aktive Bänder …). Wo der
Stand etwas anderes zeigt als eine Abnahme, wird das benannt und dem User
vorgelegt — nicht still entschieden.

**Historische Quellenregel bis zum neueren Entscheid vom 25.08.: eine
Figma-Datei `Nakama-Design`.**
(https://www.figma.com/design/NPCQYSkoZEd4Av0NlKxBOd/Nakama-Design?node-id=0-1,
Key `NPCQYSkoZEd4Av0NlKxBOd`). User-Wort 22.08.: „das ist das neue
verbindliche und einzige Depot indem sich die aktuellsten Designs befinden";
am 24.08. bekräftigt: „die sind IMMER in diesem figma, nirgends wo anders.
hier ist immer das aktuellste". Helle und dunkle Varianten sind geplant,
„stand jetzt aber erstmal nur dunkel"
(`abnahmen/2026-08-24-figma-depot-immer-aktuell.md`). Verbindlich ist
je App die **dunkle** Fassung; helle Varianten liegen dort in Arbeit und sind
noch keine Vorgabe. Claude holt die Stände per Figma-MCP in die Truhe
(`assets/figma/LIES-MICH.md`). (Der Figma-Dateiname `Nakama-Design` bleibt —
er bezeichnet die Figma-Datei, nicht mehr ein Repo.)

Die Oberflächen (**seit 23.08. zwei Apps, seit 25.08. drei aktuelle
Referenzbilder** — Entscheide
`abnahmen/2026-08-23-gen-eq-zentrale.md` und
`abnahmen/2026-08-27-arbeitsnamen-und-probeeq-doppelrolle.md`):

1. **Nakama Gen** — Hub/„Studio" mit zwei Oberflächen. „Overview" und
   „EQ-Zentrale" beziehungsweise „EQ Center" sind nur Arbeits- und
   Platzhalternamen. Die aktuelle Fläche 1 ist
   `assets/rework-basis-2026-08-25/gen-page-1-overview.png`; die aktuelle
   Fläche 2 ist `assets/rework-basis-2026-08-25/gen-page-2-eq-center.png`.
   Beide sind die sichtbare Grundlage für den großen Rework. Frühere Depot-
   Nodes (`25:444`, `28:994`) und Exporte bleiben Verlauf. Funktionale
   Spielregeln, die ein Bild nicht beweisen kann, stehen weiterhin in
   `docs/ui-spielregeln-eq-zentrale.md` und den neueren Abnahmen.
2. **Nakama Probeeq** — DIE Bus-Sonde mit Doppelrolle. Sie misst passiv und
   beliefert Gen; bei zugeschaltetem EQ führt dieselbe Instanz Gens
   Fernsteuerung auf ihrem Bus aus. Suna ist als App-Name durch Probeeq
   ersetzt. Die vollständige EQ-Bedienung liegt ausschließlich in Gen;
   Probeeq besitzt keinen lokalen Voll-Editor. Die aktuelle kompakte Referenz ist
   `assets/rework-basis-2026-08-25/probeeq.png`. Der Suna-Knoten `6:2864`,
   die frühere große Probeeq-Vollfläche und alle Suna-Exporte sind nur noch
   Verlauf und dürfen nicht als aktuelle visuelle Basis zitiert werden.

Die Projektquellen — vollständig, nichts von außen dazuholen:

- **FUNKTION UND BEDIENUNG NEU ABLEITEN:**
  `docs/funktions-und-bedien-blueprint.md` — aktueller designneutraler Vertrag
  aus Implementierungsplan, Wissensarchiv und geprüftem Marktvergleich.
- **HISTORISCHE VISUELLE VORGABE** (nicht in den neuen Blueprint übernehmen):
  die drei Bilder unter
  `assets/rework-basis-2026-08-25/`, gemäß
  `abnahmen/2026-08-25-rework-referenz-drei-designs.md`.
- **WAS** jede Oberfläche zeigen muss: `docs/oberflaechen-spezifikation.md`
- **WAS DIE TECHNIK VERLANGT** (Spielregeln der EQ-Zentrale, für die
  Arbeit an Gen Seite 1+2): `docs/ui-spielregeln-eq-zentrale.md`
- **WIE** gearbeitet wird (UX vor UI, Stufen 1–7, Schlusscheck):
  `docs/CLAUDE-UX-UI-ARBEITSKERN.md` — bei jeder Designaufgabe befolgen
- **WAS SICHTBAR SEIN DARF**: `docs/DESIGN-GESETZE.md` — Progressive
  Disclosure, Shneiderman-Mantra, Overview+Detail / Focus+Context / Zoom,
  Aufmerksamkeitsgesetze. Beim Übersetzen der Prüfmaßstab für Zustände und
  Grenzfälle, die der Stand nicht zeigt.
- **AKTUELLER DESIGN-ARBEITSWEG**: `docs/arbeitsplan.md` — die frühen
  Discovery-, Wireframe- und Graybox-Phasen sind abgeschlossen. Den nächsten
  Produktauftrag bestimmt `../docs/PLAN-STAND.md`, nicht eine alte
  Design-Phasennummer.
- **Nachschlagewerk** (Quellenkritik, Wahrnehmung, Licht/Material):
  `docs/UX-UI-WISSENSBASIS-FUER-CLAUDE.md`
- **Werkzeuge:** `docs/werkzeugplan.md`; lebende Blätter und Messbank in
  `werkzeug/` (`werkzeug/LIES-MICH.md`, `werkzeug/PRUEFLISTE.md`). Der alte
  `docs/sondenplan.md` vermisst den verworfenen lokalen Voll-Editor und dient
  nur noch der Spurensuche, nicht als Arbeitsauftrag.

Bis zum Funktionsneustart kam die Gestaltung aus den drei Rework-Bildern. Die
grundlegende Struktur-, Skizzen- und Graybox-Phase war der erste Schritt am
Projektanfang und ist abgeschlossen; sie ist kein wiederkehrendes Gate für
die aktuelle Arbeit. Was die Bilder nicht zeigen, wird gezielt am konkreten
Verhalten geklärt, ohne den Anfangsprozess neu zu starten
(`abnahmen/2026-08-25-skizzen-vor-figma.md`, Nachtrag 26.08.).

**Historischer Maßstab bis zum Funktionsneustart:** Die Umsetzung in FL Studio
folgte der jeweils aktuellen User-Vorgabe und ihrer dokumentierten Abnahme.
Seit dem 25.08.2026 waren dafür die drei Rework-Bilder maßgeblich; der frühere
Figma-Stand und sein Pixelvergleich waren Verlauf
(`abnahmen/2026-08-22-erfolgskriterium-exakt-wie-figma.md`). Beweisziel und
Vergleichsgröße werden aus dem aktuellen Auftrag abgeleitet, nicht aus diesem
historischen Kriterium.

## Harte Regeln (nicht verhandelbar)

1. **Aktuelle Phase zuerst.** Für die funktionale UI-Architektur seit
   31.08.2026 sind Blueprint und neue datierte Abnahmen die erste
   Anlaufstelle. `assets/` und die drei Bilder unter
   `assets/rework-basis-2026-08-25/` bleiben unverändert als Verlauf erhalten,
   liefern in dieser Phase aber weder Layout noch Stil. Erst nach einem
   ausdrücklichen Wechsel in die visuelle Phase wird die dann vom User
   benannte Assetquelle wieder verbindlich.
2. **Ein Schritt pro Runde.** Genau eine Frage öffnen, am lebenden Blatt
   zeigen, User-Urteil einholen, festhalten — dann die nächste. Keine
   Groß-Lieferungen mit zehn gleichzeitigen Entscheidungen.
3. **Der User entscheidet als Projektleiter die visuelle Richtung, du
   übersetzt.** Sie steht in den aktuellen drei Rework-Bildern und neueren
   ausdrücklichen User-Entscheiden. Varianten sind nur dort nötig, wo diese
   Vorgaben eine konkrete Frage offenlassen (ein Zustand, den kein Stand
   zeigt; ein Grenzfall, der das Bild sprengt). Nichts Grundsätzliches
   (Stilrichtung, Metapher, Farbwelt) ohne sein Wort festlegen oder wieder
   aufmachen. Eine konkrete fehlende oder unklare Vorstellung wird einzeln
   erfragt; der abgeschlossene Anfangsprozess wird dafür nicht neu gestartet.
   Offene Fragen an den User stehen in
   `../docs/plan/fragen.json` und werden mit dem Skill
   **`/fragen`** gestellt — er hält die Antwort wörtlich fest und arbeitet
   sie ein (`../docs/plan/LIES-MICH.md`). Die Briefing-Seite ist seit dem
   23.08.2026 abgeschafft.
4. **Abnahmen sind bindend.** Jedes User-Urteil sofort als Datei in
   `abnahmen/` festhalten (Format dort). Festgelegtes wird nicht ohne
   neues User-Wort erneut zur Diskussion gestellt — genau das erzeugt
   die „Ausbessern-Zeremonie", die dieses Projekt verhindern soll. Ein
   neuer ausdrücklicher User-Entscheid oder eine neue User-Referenz kann den
   älteren Stand ersetzen. Produktweite Entscheide stehen zusätzlich mit
   Datum + Zitat an der dafür bezeichneten Stelle gemäß Wurzel-`CLAUDE.md`.
5. **Editor-Funktionen für den User sind Pflicht.** Jedes Blatt bekommt
   ein eingebautes Bedien-Panel: Look-Parameter live verstellbar (Farben,
   Typo-Größen, Dichte, Radien, Abstände), alle Zustände durchschaltbar,
   Varianten vergleichbar. Der User gestaltet MIT, nicht nur per Zuruf.
6. **Ehrliche Blätter.** In 2 Sekunden ablesbar ist der Maßstab. Nie
   einen Zustand zeigen, den die Spezifikation nicht kennt. Demo-Daten
   sind erlaubt, aber realistisch UND sichtbar als Demo gekennzeichnet.
7. **Kein Kontext-Import aus dem Verlauf.** Für die aktuelle
   Funktionsarchitektur zählen der Blueprint und neuere ausdrückliche
   User-Entscheide. Die drei Rework-Bilder, frühere Figma-Stände sowie
   `docs/archiv/`, `eq-copilot/design/archive/`, die geparkte Prisma-Studie
   (`eq-copilot/design/prisma-studie/`) und das Material-Kit-Provisorium
   (`eq-copilot/design/` Wurzel) sind Verlauf. Aus ihnen wird keine
   Anordnung, Richtung, Metapher oder Farbwelt geholt. Der Wahrheitskern in
   `CLAUDE.md` gilt der Produktwahrheit; die neue visuelle Richtung gehört
   einem späteren ausdrücklichen User-Schritt.
8. **Nur soviel sichtbar wie der aktuelle Handgriff braucht.** Alles
   andere ist einen Schritt entfernt — nicht weg, aber nicht da.
   Beim Übersetzen: jedes Element des Stands muss beantworten, welchen
   Handgriff es gerade bedient; die Aufgabenliste nach Häufigkeit ist dafür
   die Prüfliste. Varianten — wo es sie noch gibt — unterscheiden sich in
   der **Disclosure-Mechanik** (Schritt-für-Schritt, Focus+Context,
   Overview+Detail, semantischer Zoom, nur-auf-Abruf), nicht in der
   Kastenanordnung. Begründung und Grenzen: `docs/DESIGN-GESETZE.md`.
   Merksatz gegen die naheliegende Ausrede: **nach Häufigkeit der Aufgabe
   teilen, nicht nach Können des Nutzers.**

## Werkzeuge

- **Playwright** (Browser-MCP): jedes gezeigte Ergebnis mit Screenshot
  belegen — „ausgeführt und gesehen", nie „sollte so aussehen".
- **/design**, **/dataviz**: für jede Diagramm-/Meter-Gestaltung (Heatmap,
  EQ-Kurve, Pegel) VOR dem Bauen laden — als Handwerk für die Übersetzung,
  nicht als Quelle einer Richtung.
- **/diverge**: nur bei echten Gabelungen, die die aktuelle User-Vorgabe
  offenlässt.
- **/interview**: nur wenn der User ausdrücklich eine neue Konzeptphase
  öffnet oder eine neue Vorstellung von Grund auf erarbeiten will; nicht als
  Pflichtschleife für den laufenden Rework.
- **Hooks:** Es gibt keinen Design-Primer und kein Design-Gate beim normalen
  Sessionstart. `.claude/settings.json` injiziert den kompakten Nakama-Primer
  nur nach einer Kontextkomprimierung und schützt Git-Befehle mit dem
  `git-riegel.sh`. Designkontext kommt aus diesem Router, der aktuellen
  Abnahme und dem konkreten Auftrag.
- Prototyp = statisches HTML/CSS/JS in `prototyp/`, ohne Build-Kette,
  per Doppelklick lauffähig. Seine bestehende Grundstruktur wird nicht
  erneut als offene Anfangsentscheidung behandelt.

## Arbeitsweise

- **Die App ist auf Englisch.** User-Wort vom 2026-08-21: „Englisch – mein
  Wort" (`abnahmen/2026-08-21-designvertrag.md`). Jeder Text, der im Produkt
  steht — Beschriftungen, Werte, Einheiten, Zustände, Meldungen — ist
  englisch. Das gilt auch für die Spezimen in `werkzeug/`, weil sie das
  Produkt zeigen. Das Werkzeug-Chrome darum herum (Fragen an den User)
  bleibt deutsch.
- Docs, Abnahmen, Commits und Gespräch auf Deutsch.
- Die aktuellen Apps heißen Gen und Probeeq. Suna sowie Main /
  Active-Probe-Editor / Passive-Kachel sind Legacy- oder Verlaufsbegriffe.
  Overview und EQ-Zentrale/EQ Center benennen die zwei Gen-Flächen nur
  vorläufig und werden nicht als abgenommene Produktnamen behandelt.
- Kleine, benannte Commits nach jedem abgenommenen Schritt — immer mit
  explizitem Pathspec, nie `git add -A` (parallele Sessions).
- Sessionende: eine Zeile in `abnahmen/` was offen blieb — kein
  Wissenstransfer über Gedächtnis. Am Planstand ist **nichts** nachzuziehen:
  er wird gerechnet (`docs/plan/LIES-MICH.md`, seit 23.08.2026).
