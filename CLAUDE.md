# NAKAMA-DESIGN

Design- und UI-Projekt für **Nakama**, eine Plugin-Familie für FL Studio,
die Musiker beim Mischen berät (sie misst und empfiehlt — sie mischt nicht
selbst). Hier entsteht das visuelle Design als **Web-Prototyp im Browser**.
Es wird KEIN Plugin-Code geschrieben; der technische Unterbau lebt in einem
anderen Repo und ist ausdrücklich nicht Thema dieses Projekts.

## Dein Job

Einen Web-Design-Prototyp für **drei Oberflächen** bauen und mit dem User
am lebenden Blatt verfeinern:

1. **Main** — die große Arbeitsfläche des Master-Plugins (Quellen,
   Befunde, Assistent).
2. **Active-Probe-Editor** — ein vollwertiger 8-Band-EQ-Editor mit
   Nakama-Zusätzen.
3. **Passive-Probe-Kachel** — eine kleine Status-/Identitätskachel.

Die fünf Projektquellen — vollständig, nichts von außen dazuholen:

- **WAS** jede Oberfläche zeigen muss: `docs/oberflaechen-spezifikation.md`
- **WIE** gearbeitet wird (UX vor UI, Stufen 1–7, Schlusscheck):
  `docs/CLAUDE-UX-UI-ARBEITSKERN.md` — bei jeder Designaufgabe befolgen
- **WAS SICHTBAR SEIN DARF**: `docs/DESIGN-GESETZE.md` — Progressive
  Disclosure, Shneiderman-Mantra, Overview+Detail / Focus+Context / Zoom,
  Aufmerksamkeitsgesetze. **Vor jeder Anordnungs- und
  Sichtbarkeitsentscheidung lesen.**
- **IN WELCHER REIHENFOLGE**: `docs/arbeitsplan.md` — dort steht der
  konkrete Einstieg (Phase 0: `/interview designvertrag`, vor jedem Bauen)
- **Nachschlagewerk** (Quellenkritik, Wahrnehmung, Licht/Material):
  `docs/UX-UI-WISSENSBASIS-FUER-CLAUDE.md`

Alles Gestalterische ist offen und wird MIT dem User entwickelt.

## Harte Regeln (nicht verhandelbar)

1. **Assettruhe zuerst.** `assets/` ist die erste Anlaufstelle vor JEDER
   Gestaltungsentscheidung. Was dort liegt, wird verwendet — nicht neu
   erfunden. Fehlt etwas, den User fragen statt improvisieren. Die Truhe
   wird nie ungefragt verändert.
2. **Ein Schritt pro Runde.** Genau eine gestalterische Frage öffnen,
   am lebenden Blatt zeigen, User-Urteil einholen, festhalten — dann die
   nächste. Keine Groß-Lieferungen mit zehn gleichzeitigen Entscheidungen.
3. **Der User entscheidet die Richtung, du übersetzt.** Bei echten
   Gabelungen 2–3 Varianten NEBENEINANDER zeigen statt selbst zu wählen.
   Nichts Grundsätzliches (Stilrichtung, Metapher, Farbwelt) ohne sein
   Wort festlegen oder wieder aufmachen. Fehlende oder unklare
   Vorstellung wird per **`/interview <phase>`** erfragt — Pflicht zu
   Beginn jeder Arbeitsplan-Phase, jederzeit zusätzlich einsetzbar.
4. **Abnahmen sind bindend.** Jedes User-Urteil sofort als Datei in
   `abnahmen/` festhalten (Format dort). Festgelegtes wird nicht ohne
   neues User-Wort erneut zur Diskussion gestellt — genau das erzeugt
   die „Ausbessern-Zeremonie", die dieses Projekt verhindern soll.
5. **Editor-Funktionen für den User sind Pflicht.** Jedes Blatt bekommt
   ein eingebautes Bedien-Panel: Look-Parameter live verstellbar (Farben,
   Typo-Größen, Dichte, Radien, Abstände), alle Zustände durchschaltbar,
   Varianten vergleichbar. Der User gestaltet MIT, nicht nur per Zuruf.
6. **Ehrliche Blätter.** In 2 Sekunden ablesbar ist der Maßstab. Nie
   einen Zustand zeigen, den die Spezifikation nicht kennt. Demo-Daten
   sind erlaubt, aber realistisch UND sichtbar als Demo gekennzeichnet.
7. **Kein Kontext-Import.** Nicht in anderen Ordnern, Repos oder alten
   Design-Dokumenten recherchieren — der Kontext dieses Ordners ist
   absichtlich vollständig und absichtlich frei von Alt-Ideen. (Es
   existiert daneben eine separate Begleit-App mit eigener Design-Sprache;
   sie ist NICHT Teil dieses Projekts und kein Referenzmaterial.)
8. **Nur soviel sichtbar wie der aktuelle Handgriff braucht.** Alles
   andere ist einen Schritt entfernt — nicht weg, aber nicht da.
   Vor jedem Wireframe die Aufgabenliste nach Häufigkeit schreiben; jedes
   Element muss beantworten, welchen Handgriff es gerade bedient. Varianten
   unterscheiden sich in der **Disclosure-Mechanik** (Schritt-für-Schritt,
   Focus+Context, Overview+Detail, semantischer Zoom, nur-auf-Abruf), nicht
   in der Kastenanordnung. Begründung und Grenzen: `docs/DESIGN-GESETZE.md`.
   Merksatz gegen die naheliegende Ausrede: **nach Häufigkeit der Aufgabe
   teilen, nicht nach Können des Nutzers.**

## Werkzeuge

- **Playwright** (Browser-MCP): jedes gezeigte Ergebnis mit Screenshot
  belegen — „ausgeführt und gesehen", nie „sollte so aussehen".
- **/design**, **/dataviz**: für Layout-Entwürfe und jede Diagramm-/
  Meter-Gestaltung (Heatmap, EQ-Kurve, Pegel) VOR dem Bauen laden.
- **/diverge**: nur bei echten Gabelungen, nicht für Alltagsfragen.
- **/interview**: der Fragenkatalog dieses Projekts (Phasen:
  designvertrag · struktur · richtung · komponente · zustaende ·
  abnahme) — die Vorstellung des Users erfassen, wörtlich protokollieren.
- Prototyp = statisches HTML/CSS/JS in `prototyp/`, ohne Build-Kette,
  per Doppelklick lauffähig. Struktur (ein Blatt pro Oberfläche oder
  Tabs) ist die erste Entscheidung — mit dem User. Eine mechanische
  Schleuse blockt `prototyp/`-Writes, solange keine Datei mit
  **`designvertrag`** im Namen in `abnahmen/` liegt (Arbeitsplan Phase 0).
  Andere Abnahmen öffnen das Tor nicht — auch fünfzehn nicht. Sie greift
  für `Write`/`Edit` **und für `Bash`**; Gegenprobe:
  `bash .claude/hooks/schleusen-probe.sh`.

## Arbeitsweise

- **Die App ist auf Englisch.** Jeder Text, der im Produkt steht —
  Beschriftungen, Werte, Einheiten, Zustände, Meldungen — ist englisch.
  Das gilt auch für die Spezimen in `werkzeug/`, weil sie das Produkt
  zeigen. Das Werkzeug-Chrome darum herum (Fragen an den User) bleibt
  deutsch.
- Docs, Abnahmen, Commits und Gespräch auf Deutsch.
- Kleine, benannte Commits nach jedem abgenommenen Schritt.
- Sessionende: eine Zeile in `abnahmen/` was offen blieb — kein
  Wissenstransfer über Gedächtnis.
