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

## Dein Job

**Die aktuelle User-Vorgabe übersetzen, nicht Design erfinden.** Seit dem
neuesten User-Entscheid vom 25.08. sind genau drei gelieferte PNGs die
**aktuelle visuelle Wahrheit in progress** und die Grundlage des späteren
großen Reworks:

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
`abnahmen/2026-08-23-gen-eq-zentrale.md`; Suna ist in Probeeq aufgegangen):

1. **Nakama Gen** — Hub/„Studio". Die aktuelle Seite 1 ist
   `assets/rework-basis-2026-08-25/gen-page-1-overview.png`; die aktuelle
   Seite 2 ist `assets/rework-basis-2026-08-25/gen-page-2-eq-center.png`.
   Beide sind die sichtbare Grundlage für den großen Rework. Frühere Depot-
   Nodes (`25:444`, `28:994`) und Exporte bleiben Verlauf. Funktionale
   Spielregeln, die ein Bild nicht beweisen kann, stehen weiterhin in
   `docs/ui-spielregeln-eq-zentrale.md` und den neueren Abnahmen.
2. **Nakama Probeeq** — DIE Sonde auf den Bussen. Ihre aktuelle kompakte
   Referenz ist `assets/rework-basis-2026-08-25/probeeq.png`. Der Suna-Knoten
   `6:2864`, die frühere große Probeeq-Vollfläche und alle Suna-Exporte sind
   nur noch Verlauf und dürfen nicht als aktuelle visuelle Basis zitiert
   werden.

Die Projektquellen — vollständig, nichts von außen dazuholen:

- **DIE VORGABE** (wie es aussieht): die drei Bilder unter
  `assets/rework-basis-2026-08-25/`, gemäß
  `abnahmen/2026-08-25-rework-referenz-drei-designs.md`.
- **WAS** jede Oberfläche zeigen muss: `docs/oberflaechen-spezifikation.md`
- **WAS DIE TECHNIK VERLANGT** (Spielregeln der EQ-Zentrale, für die
  Figma-Arbeit an Gen Seite 1+2): `docs/ui-spielregeln-eq-zentrale.md`
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
- **Werkzeug-Pläne:** `docs/sondenplan.md`, `docs/werkzeugplan.md`;
  lebende Blätter und Messbank in `werkzeug/` (`werkzeug/LIES-MICH.md`,
  `werkzeug/PRUEFLISTE.md`).

Die Gestaltung kommt derzeit aus den drei aktuellen Rework-Bildern. Offen
ist, was diese Bilder nicht zeigen — und das wird zuerst als Skizze, bei
Bedarf als Graybox und erst danach in Figma mit dem User geklärt
(`abnahmen/2026-08-25-skizzen-vor-figma.md`).

**Maßstab (User-Wort 2026-08-22): die Apps sehen in FL Studio exakt aus wie
in Figma.** Zwischenbilder zählen nicht. Darum: **Statisches wird gebacken,
Dynamisches gezeichnet** — Wortmarken (Shader), Schale/Glas/Kinn und
Leuchtsäume als Figma-Exporte je Skalierungsstufe, Geist/Geist Mono
eingebettet, der Prototyp mit denselben Assets wie das Plugin; Beweis ist der
1:1-Pixelvergleich gegen den Export des Users
(`abnahmen/2026-08-22-erfolgskriterium-exakt-wie-figma.md`).

## Harte Regeln (nicht verhandelbar)

1. **Assettruhe zuerst.** `assets/` ist die erste Anlaufstelle vor JEDER
   Gestaltungsentscheidung; aktuell sind die drei Bilder unter
   `assets/rework-basis-2026-08-25/` ihr wichtigster Inhalt. Was dort liegt,
   wird verwendet — nicht neu
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
   Arbeitsplan-Phase, jederzeit zusätzlich einsetzbar. Offene Fragen an den
   User stehen in `docs/plan/fragen.json` und werden mit dem Skill
   **`/fragen`** gestellt — er hält die Antwort wörtlich fest und arbeitet
   sie ein (`docs/plan/LIES-MICH.md`). Die Briefing-Seite ist seit dem
   23.08.2026 abgeschafft.
4. **Abnahmen sind bindend.** Jedes User-Urteil sofort als Datei in
   `abnahmen/` festhalten (Format dort). Festgelegtes wird nicht ohne
   neues User-Wort erneut zur Diskussion gestellt — genau das erzeugt
   die „Ausbessern-Zeremonie", die dieses Projekt verhindern soll. Ein
   neuer Figma-Stand ist neues User-Wort. Entscheide, die das ganze
   Produkt betreffen, stehen zusätzlich mit Datum + Zitat im Register der
   Wurzel-`CLAUDE.md`.
5. **Editor-Funktionen für den User sind Pflicht.** Jedes Blatt bekommt
   ein eingebautes Bedien-Panel: Look-Parameter live verstellbar (Farben,
   Typo-Größen, Dichte, Radien, Abstände), alle Zustände durchschaltbar,
   Varianten vergleichbar. Der User gestaltet MIT, nicht nur per Zuruf.
6. **Ehrliche Blätter.** In 2 Sekunden ablesbar ist der Maßstab. Nie
   einen Zustand zeigen, den die Spezifikation nicht kennt. Demo-Daten
   sind erlaubt, aber realistisch UND sichtbar als Demo gekennzeichnet.
7. **Kein Kontext-Import aus dem Verlauf.** Für Gestaltung zählen nur
   dieser Ordner und die Figma-Stände des Users. Die übrigen Design-Orte
   des Repos sind Verlauf, kein Referenzmaterial: `docs/archiv/`,
   `eq-copilot/design/archive/`, die geparkte Prisma-Studie
   (`eq-copilot/design/prisma-studie/`) und das Material-Kit-Provisorium
   (`eq-copilot/design/` Wurzel). Aus ihnen wird keine Richtung, Metapher
   oder Farbwelt geholt (User 21.08.: „alle alten sind alt"). Der
   Wahrheitskern in `CLAUDE.md` gilt der Produktwahrheit, nicht der
   Gestaltung.
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
- **/interview**: der Fragenkatalog dieses Ordners
  (`.claude/skills/interview/SKILL.md` an der Repo-Wurzel; Phasen:
  designvertrag · struktur · ~~richtung~~ (entfallen seit 2026-08-21: die
  Richtung kommt aus Figma) · komponente · zustaende · abnahme) — die
  Vorstellung des Users erfassen, wörtlich protokollieren.
- **Hooks** (in `tools/hooks/`, verdrahtet in `.claude/settings.json` der
  Repo-Wurzel): `design-primer.sh` misst zu Sessionbeginn Truhe und Abnahmen.
  Die frühere `prototyp-schleuse.sh` wurde am 24.08.2026 stillgelegt: Ihr
  einziges Gate war mit dem Designvertrag vom 21.08. dauerhaft erfüllt und
  erzeugte danach nur noch Start- und Werkzeuglast. Der Prototyp ist frei.
  `schleusen-probe.sh` prüft weiterhin die aktive Kreativ-Schleuse.
- Prototyp = statisches HTML/CSS/JS in `prototyp/`, ohne Build-Kette,
  per Doppelklick lauffähig. Struktur (ein Blatt pro Oberfläche oder
  Tabs) ist die erste Entscheidung — mit dem User.

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
  Wissenstransfer über Gedächtnis. Am Planstand ist **nichts** nachzuziehen:
  er wird gerechnet (`docs/plan/LIES-MICH.md`, seit 23.08.2026).
