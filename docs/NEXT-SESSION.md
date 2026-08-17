# NEXT-SESSION — Einstieg für die nächste Runde

> Stand: 2026-08-17, Session-Ende. Diese Datei ist der Schnellstart;
> Tiefe in `docs/design-stand.md` und `docs/geschmacksprofil.md`.

## Der eine nächste Schritt

**Prisma-Material-Probestill rendern** (User hat zugestimmt, wartet darauf):

1. Blender 5.1 liegt unter `%ProgramFiles%\Blender Foundation\Blender 5.1`
   (nicht im PATH). Headless per Skript, **Cycles auf CPU** — niemals GPU
   (Arc-A770-Lüfter-Landmine, CLAUDE.md).
2. Dreiseitiges Prisma, echtes Glas mit Dispersion/Kaustik, EIN Still
   (keine Animation, keine Daten) — komponiert über dem User-Hintergrund
   (Screenshot der Unicorn-Szene oder dunkler Platzhalter).
3. **Glas-Anmutung ist noch offen** — User wollte ggf. noch sagen: klar/
   museal · dunkel-rauchig · frostig (passend zu Winter Nexus). Falls
   keine Antwort kam: 2–3 Material-Varianten als Stills anbieten.
4. Der User beurteilt am Still NUR „Gimmick oder ernstzunehmend". Erst
   nach Abnahme: Flächen-Inhalt + Bündel-Inhalt gemeinsam entwickeln,
   dann Übergangssequenzen.

## Warum Prisma (Kurzform)

User-autorisiertes Konzept: 3 Flächen = Tiefen/Mitten/Höhen, Drehen =
Selektion, Klick = Bündeln (Newton-Rekombination = Analyse↔Synthese),
kompakter Dauerbegleiter statt Analyzer-Fenster. Darstellung:
Film-Compositing (vorgerenderte Cycles-Passes + live Daten-Ebene in
P01-Punktästhetik + Unicorn-Hintergrund als Ebene 0). Details:
`design-stand.md`, Abschnitt „AKTIVE RICHTUNG".

## Nicht verhandelbar (aus dieser Session gelernt)

- **KREATIV-SCHLEUSE:** Nie wieder losbauen ohne mit dem User
  ausgearbeitete Idee. Hook blockt Write/Edit unter `eq-copilot/design/`
  ohne frische `.claude/kreativ-freigabe.md` (Marker gilt 24 h — für die
  nächste Session neu schreiben, mit dem, was der User freigibt!).
- **Geschmacksprofil lesen** (`docs/geschmacksprofil.md`) vor jeder
  visuellen Entscheidung: die drei Gesetze + 3a–3c + Meta-Erkenntnis
  (Vision kommt vom User; Claudes freie Bilderfindung = Mode-Collapse;
  P01-Punktästhetik ist die einzige abgenommene).

## Infrastruktur, die steht

- `eq-copilot/design/unicorn-bruecke.html` — Unicorn-Szene + Musikdaten
  (`?projekt=6YU0kFPu0uCBf0ZOTXOC` = User-Hintergrund „Winter Nexus",
  mausreaktiv; JSON-Backup in `design/unicorn/`). Taste T = Tafel.
- `eq-copilot/design/proben/` — 4 Kalibrier-Proben + `render-probe.mjs`
  (generisch: Blatt, Name, Query, Größe, Wartezeit; Einzellauf, SwiftShader).
- 30-s-Songschleife (`songLage`/`musikDb`) — in jedem Blatt identisch
  portiert; deterministisch, `?zeit=` friert ein.
- Geparkt, unangetastet: Tiefenfeld + Bauplan 2.0 + Kunstwerk-Studie
  (drei Welten) — nie ungefragt reaktivieren.

## Offene kleine Punkte

- NAK-11: Unicorn-Lizenz („Legend"-Plan für Offline-JSON) vor
  Plugin-Auslieferung klären — Design-Phase unkritisch.
- Nakama-Repo hat kein Git-Remote (Push unmöglich) — bei Gelegenheit
  mit dem User klären, ob eines angelegt werden soll.
