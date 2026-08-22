# NAKAMA-DESIGN

Design- und UI-Projekt für **Nakama**, eine Plugin-Familie für FL Studio:
drei Apps mit **einer** Design-Identität. Gen und Suna beraten beim Mischen
(sie messen und empfehlen), Probeeq ist der EQ, der ausführt. Hier entsteht
das Design als **Web-Prototyp im Browser**. Es wird KEIN Plugin-Code
geschrieben; der technische Unterbau lebt in einem anderen Repo und ist
nicht Thema dieses Projekts. Die Produktwahrheit (was die drei Apps sind und
dürfen) steht dort in `Nakama/CLAUDE.md`, Abschnitt Wahrheitskern — hier
wird sie nicht abgeschrieben, nur verwiesen.

## Dein Job

**Figma übersetzen, nicht Design erfinden.** Der User gestaltet das finale
Design in Figma; seine Stände liegen in `assets/figma/` und sind die Vorgabe
(Designvertrag 2026-08-21: „Figma ist Quelle; Repo setzt um"). Dieses Repo
macht daraus **lebende Blätter** und prüft, was ein Bild nicht prüfen kann:
**Zustände** (frisch / veraltet / getrennt, Messung, degradiert, Draft,
Bypass …), **Größen** (bei 1:1 auf den abgenommenen Maßen) und
**Grenzfälle** (16 Quellen, alles getrennt, acht aktive Bänder …). Wo der
Stand etwas anderes zeigt als eine Abnahme, wird das benannt und dem User
vorgelegt — nicht still entschieden.

Die drei Oberflächen:

**Die Quelle ist seit 2026-08-22 EINE Figma-Datei: `Nakama-Design`**
(https://www.figma.com/design/NPCQYSkoZEd4Av0NlKxBOd/Nakama-Design?node-id=0-1, Key `NPCQYSkoZEd4Av0NlKxBOd`). User-Wort: „das ist das neue verbindliche und einzige Depot indem sich die aktuellsten Designs befinden. ich habe für jedes design vor eine helle und dunke variante zu erstellen. stand jetzt aber erstmal nur dunkel“
(`abnahmen/2026-08-22-figma-depot.md`). Verbindlich ist je App die **dunkle**
Fassung; helle Varianten liegen dort in Arbeit und sind noch keine Vorgabe.
Claude holt die Stände per Figma-MCP in die Truhe (`assets/figma/LIES-MICH.md`).

1. **Nakama Gen** — die Main-App: Quellen-Übersicht, Befunde, Advisor.
   760×430. Stand: `assets/figma/2026-08-22-gen.png` (Depot `6:13`, Studie 04
   in der Probeeq-Sprache). Die 18 Lesarten dazu beantwortet der User im
   Nakama-Hub (`abnahmen/2026-08-21-gen-auftrag-figma.md`). Der Stand vom
   21.08. (cyan) ist Verlauf.
2. **Nakama Probeeq** — die aktive Sonde: ein **vollwertiger, hochwertiger
   EQ**, der Anweisungen von Gen direkt umsetzt **und** ganz normal manuell
   bedienbar ist. Muss allein funktionieren, auch ohne Gen. 700×420.
   Stand: `assets/figma/2026-08-22-probeeq.png` (Depot `6:1906`).
3. **Nakama Suna** — die passive Sonde: eine kleine Status-/Identitätskachel.
   260×84. Stand: `assets/figma/2026-08-22-suna.png` (Depot `6:2629`) —
   entworfen von Claude in der Probeeq-Sprache (Studie 03, Node 6039:2), vom
   User exportiert und damit User-Wort; die elf offenen Punkte der Übersetzung
   stehen in `abnahmen/2026-08-21-suna-auftrag-figma.md`.

Die Projektquellen — vollständig, nichts von außen dazuholen:

- **DIE VORGABE** (wie es aussieht): `assets/figma/` — je Stand eine datierte
  Datei, beschrieben in `assets/figma/LIES-MICH.md`; der neueste gilt.
- **WAS** jede Oberfläche zeigen muss: `docs/oberflaechen-spezifikation.md`
- **WIE** gearbeitet wird (UX vor UI, Stufen 1–7, Schlusscheck):
  `docs/CLAUDE-UX-UI-ARBEITSKERN.md` — bei jeder Designaufgabe befolgen
- **WAS SICHTBAR SEIN DARF**: `docs/DESIGN-GESETZE.md` — Progressive
  Disclosure, Shneiderman-Mantra, Overview+Detail / Focus+Context / Zoom,
  Aufmerksamkeitsgesetze. Beim Übersetzen der Prüfmaßstab für Zustände und
  Grenzfälle, die der Stand nicht zeigt.
- **IN WELCHER REIHENFOLGE**: `docs/arbeitsplan.md` — Phase 0 (Designvertrag)
  ist seit 2026-08-21 erledigt: `abnahmen/2026-08-21-designvertrag.md`.
  Der nächste Schritt steht dort.
- **Nachschlagewerk** (Quellenkritik, Wahrnehmung, Licht/Material):
  `docs/UX-UI-WISSENSBASIS-FUER-CLAUDE.md`

Die Gestaltung kommt aus Figma. Offen ist, was Figma nicht zeigt — und das
wird am lebenden Blatt MIT dem User geklärt.

**Maßstab (User-Wort 2026-08-22): die Apps sehen in FL Studio exakt aus wie
in Figma.** Zwischenbilder zählen nicht. Darum: **Statisches wird gebacken,
Dynamisches gezeichnet** — Wortmarken (Shader), Schale/Glas/Kinn und
Leuchtsäume als Figma-Exporte je Skalierungsstufe, Geist/Geist Mono
eingebettet, der Prototyp mit denselben Assets wie das Plugin; Beweis ist der
1:1-Pixelvergleich gegen den Export des Users
(`abnahmen/2026-08-22-erfolgskriterium-exakt-wie-figma.md`).

## Harte Regeln (nicht verhandelbar)

1. **Assettruhe zuerst.** `assets/` ist die erste Anlaufstelle vor JEDER
   Gestaltungsentscheidung; die Figma-Stände in `assets/figma/` sind ihr
   wichtigster Inhalt. Was dort liegt, wird verwendet — nicht neu
   erfunden. Fehlt etwas, den User fragen statt improvisieren. Die Truhe
   wird nie ungefragt verändert.
2. **Ein Schritt pro Runde.** Genau eine Frage öffnen, am lebenden Blatt
   zeigen, User-Urteil einholen, festhalten — dann die nächste. Keine
   Groß-Lieferungen mit zehn gleichzeitigen Entscheidungen.
3. **Der User entscheidet die Richtung, du übersetzt.** Die Richtung steht
   in Figma. 2–3 Varianten NEBENEINANDER zeigen gilt nur noch dort, wo
   Figma eine Frage offenlässt (ein Zustand, den kein Stand zeigt; ein
   Grenzfall, der das Bild sprengt). Nichts Grundsätzliches (Stilrichtung,
   Metapher, Farbwelt) ohne sein Wort festlegen oder wieder aufmachen —
   und nichts davon selbst vorschlagen. Fehlende oder unklare Vorstellung
   wird per **`/interview <phase>`** erfragt — Pflicht zu Beginn jeder
   Arbeitsplan-Phase, jederzeit zusätzlich einsetzbar.
4. **Abnahmen sind bindend.** Jedes User-Urteil sofort als Datei in
   `abnahmen/` festhalten (Format dort). Festgelegtes wird nicht ohne
   neues User-Wort erneut zur Diskussion gestellt — genau das erzeugt
   die „Ausbessern-Zeremonie", die dieses Projekt verhindern soll. Ein
   neuer Figma-Stand ist neues User-Wort.
5. **Editor-Funktionen für den User sind Pflicht.** Jedes Blatt bekommt
   ein eingebautes Bedien-Panel: Look-Parameter live verstellbar (Farben,
   Typo-Größen, Dichte, Radien, Abstände), alle Zustände durchschaltbar,
   Varianten vergleichbar. Der User gestaltet MIT, nicht nur per Zuruf.
6. **Ehrliche Blätter.** In 2 Sekunden ablesbar ist der Maßstab. Nie
   einen Zustand zeigen, den die Spezifikation nicht kennt. Demo-Daten
   sind erlaubt, aber realistisch UND sichtbar als Demo gekennzeichnet.
7. **Kein Kontext-Import.** Nicht in anderen Ordnern, Repos oder alten
   Design-Dokumenten nach Gestaltung recherchieren — der Kontext dieses
   Ordners ist absichtlich vollständig und absichtlich frei von Alt-Ideen.
   Die erlaubte Quelle von außen sind die Figma-Exports des Users in
   `assets/figma/`. (Der Verweis auf den Wahrheitskern in `Nakama/CLAUDE.md`
   oben gilt der Produktwahrheit, nicht der Gestaltung; die dort geparkten
   Studien und Archive sind kein Referenzmaterial.)
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
- **/diverge**: nur bei echten Gabelungen, die Figma offenlässt.
- **/interview**: der Fragenkatalog dieses Projekts (Phasen:
  designvertrag · struktur · ~~richtung~~ (entfallen seit 2026-08-21: die
  Richtung kommt aus Figma) · komponente · zustaende ·
  abnahme) — die Vorstellung des Users erfassen, wörtlich protokollieren.
- Prototyp = statisches HTML/CSS/JS in `prototyp/`, ohne Build-Kette,
  per Doppelklick lauffähig. Struktur (ein Blatt pro Oberfläche oder
  Tabs) ist die erste Entscheidung — mit dem User. Eine mechanische
  Schleuse blockt `prototyp/`-Writes, solange keine Datei mit
  **`designvertrag`** im Namen in `abnahmen/` liegt (Arbeitsplan Phase 0).
  **Seit 2026-08-21 liegt sie vor** (`abnahmen/2026-08-21-designvertrag.md`),
  die Schleuse ist offen — gemessen: Write und Bash nach `prototyp/`
  exit 0. Sie greift weiter für `Write`/`Edit` **und für `Bash`**, falls
  die Datei je verschwindet; Gegenprobe: `bash .claude/hooks/schleusen-probe.sh`
  (misst den geschlossenen Zustand gegen ein leeres Temp-Verzeichnis und den
  offenen gegen das echte `abnahmen/` — 26/26, ausgeführt 21.08.).

## Arbeitsweise

- **Die App ist auf Englisch.** User-Wort vom 2026-08-21: „Englisch – mein
  Wort" (`abnahmen/2026-08-21-designvertrag.md`). Jeder Text, der im Produkt
  steht — Beschriftungen, Werte, Einheiten, Zustände, Meldungen — ist
  englisch. Das gilt auch für die Spezimen in `werkzeug/`, weil sie das
  Produkt zeigen. Das Werkzeug-Chrome darum herum (Fragen an den User)
  bleibt deutsch.
- Docs, Abnahmen, Commits und Gespräch auf Deutsch.
- Die neuen Namen (Gen · Probeeq · Suna) in allem, was neu geschrieben
  wird; alte Texte mit Main / Active-Probe-Editor / Passive-Kachel meinen
  dieselben drei Oberflächen und werden nicht rückwirkend umgeschrieben.
- Kleine, benannte Commits nach jedem abgenommenen Schritt — immer mit
  explizitem Pathspec, nie `git add -A` (parallele Sessions).
- Sessionende: eine Zeile in `abnahmen/` was offen blieb — kein
  Wissenstransfer über Gedächtnis.
