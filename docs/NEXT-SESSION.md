# NEXT-SESSION — Einstieg für die nächste Runde

> **WORKSPACE-UMZUG 18.08.2026:** Alles lebt jetzt HIER
> (`Projekte\Nakama`) — eq-copilot/, broker/ (eigenständig,
> eqcop-broker.exe), tools/. FL-Repo-SHAs in älteren Einträgen unten
> gehören zur FL-Studio-Historie (bis `7964777`).

> Stand: 2026-08-17, Session-Ende. Diese Datei ist der Schnellstart;
> Tiefe in `docs/design-stand.md` und `docs/geschmacksprofil.md`.

## Der eine nächste Schritt

**ST-MAP-LIVE-REFRAKTION BAUEN (User-Freigabe 19.08., Marker
`.claude/kreativ-freigabe.md` — Wortlaut dort). Rendern bewusst in DIESE
frische Session verschoben.**

Warum: Die erste Freistellung (dreh-frei, NAK-15) ist vom User
VERWORFEN — „das prisma hat in sich die spiegelung des alten
hintergrundes eingebacken … die beleuchtung komplett falsch … billig
zusammengeklebt". Volle Analyse: `docs/geschmacksprofil.md`
(Freistellungs-Probe 19.08.) + NAK-16 in `docs/offene-punkte.md`.

Der Plan (erst EIN Probe-Still komplett beweisen, dann die 72er-Sequenz):

1. **ST-Map backen** (`prisma-material-still.py` erweitern): Plate-Fläche
   bekommt statt des Nexus-Bilds einen UV-Koordinatengradienten als
   Emission; Glas mit EINEM IOR (Mitte 1,474, keine Dispersion in der
   Karte); hohe Samples, KEIN Denoise (der Denoiser schmiert Gradienten);
   16-Bit-Präzision nötig — als 8-Bit-PNG mit Hi/Lo-Byte-Split
   (u→R+G, v→B+A). Rays, die die Plate verfehlen → Alpha 0.
2. **Glanz-Pass**: existiert schon als `--nur-glas` (Oberflächenglanz +
   Kanten gegen transparent) — wiederverwenden, ggf. Licht-Rig an die
   Nexus-Lichtwelt angleichen (Probe entscheidet).
3. **Blatt**: WebGL-Pass sampelt pro Frame den lebenden Unicorn-Canvas
   (texImage2D vom Canvas, gleiche Seite = erlaubt) durch die ST-Map;
   Dispersion optisch durch 3 leicht versetzte Abgriffe (R/G/B) im
   Shader. Darüber der Glanz-Pass, darunter ggf. Boden-Kontaktebene
   (dreh-boden bleibt brauchbar).
4. **Erfolgsmaßstab (nicht verhandelbar):** Das Glasinnere zeigt den
   LEBENDEN Hintergrund und bewegt sich mit ihm — zwei Screenshots mit
   verschiedenem Hintergrund-Stand müssen sich IM Glas unterscheiden.
   „Ähnlichkeit zum alten Frame" ist als Metrik verboten (damit wurde
   der Fehler letzte Nacht als Erfolg gemessen).

Stand bis dahin: Das Blatt zeigt als Vorgabe die BEANSTANDETE
dreh-frei-Fassung (`?frames=dreh` = alter Studio-Satz). Maschinen-Fakten:
~75 s/Frame bei 320 Samples 768×1024 CPU; Sequenz-Kamera = `KAM` im
Blatt (lens 65, −1,65/−3,53/0,95 → 0,0,0,78), NICHT die Skript-Defaults;
`--weiter` überspringt existierende Frames.

Danach weiter offen (User-Urteile): Boden-Kontaktebene an/aus (Taste B) ·
bandrelative Fahrt (Tasten 2/3) · Unicorn-Bindungen der
Drei-String-Szene (User-seitig).

Was die Messung ergab (Commit 7148248, Details `docs/design-stand.md`):

1. **lineIdx-Verdacht WIDERLEGT** — gemessen (1/60-s-Paar: nur
   Subpixel-Schimmer, 1 Pixel Vollausschlag; Noise-Floor exakt 0) und aus
   dem Code bewiesen (sichtbare Punkte enden bei 0,30·spacing, Identität
   kippt erst bei 0,5·spacing). Den notierten Shader-Umbau NICHT machen.
2. **Wahre Ursachen behoben:** stehender Prüfton (→ Fahrt, `?fahrt=0` oder
   `?ptonHz=` stellt still, Taste F) und Bin-Treppe (→ Sub-Bin-Interpolation
   in Blatt UND Brücke, befundOrt beider Seiten bitgleich verifiziert).

Offen daran: die Persistenz eines Wanderers ist ehrlich niedrig (~23 %) —
der Wirbel ist deshalb kompakt statt feldhoch. Wenn der User mehr
Vertikale will, ist das eine Datenfrage (langsamere Fahrt, längerer
Aufenthalt), keine Zeichenfrage.

**KREATIV-SCHLEUSE:** Der Marker `.claude/kreativ-freigabe.md` ist vom
18.08. 14:08 und gilt 24 h — für Design-Arbeit danach neu schreiben, mit dem,
was der User freigibt.

## Was steht (18.08., alles gemessen und committet)

- **Instrument kalibriert:** Deflection 100 → Lücke 195 %, engster Abstand
  77 %; linear; Spiegelsymmetrie 0,0 px; Mittellinie steht (719,5 bei 720,0).
- **Shader versioniert:** `prisma/schlieren-probe-field.frag` — ab jetzt die
  Quelle, Unicorn ist nur die Anzeige. Faktor 0,76 IST die Kalibrierung.
- **Punktgröße** `min(1,6·fwidth, 0,30·Abstand)` — beide Reinformen sind
  gescheitert (Vollfläche bei klein, unsichtbar bei 4K bzw. auf der
  Prismenfläche).
- **Brücke** sendet 0–1 (am Szenen-Export belegt, nicht 0–100),
  `?projekt=ilYX64xlvU811imBKfxV&band=0`.
- **Unicorn-Fallen dokumentiert** in `docs/design-stand.md`: Regler von Hand
  verschieben löst die Bindung; falsch gebundene Variable ist still.
- Szenen-Sicherung: `eq-copilot/design/unicorn/schlieren-probe-field-szene.json`

## Vorheriger Stand (P01 im Glas — Inhalt verworfen)

**Flächen-Inhalt + Bündel-Inhalt IM GESPRÄCH entwickeln** (nicht bauen!
KREATIV-SCHLEUSE: erst ausarbeiten, bis der User eine Vorstellung freigibt).
Material-Urteil ist da (18.08.): **klar** vorn, rauchig Reserve, Frost raus
(„zumindest auf den Bildern" — Anmutungs-Endabnahme am lebenden Objekt).
Kandidaten aus der User-autorisierten Richtung: Klangkörper-Silhouette ·
Balance-Ring · P01-Perkolation im Glas (einzige abgenommene Ästhetik).
Vision kommt vom User — Referenzen erfragen, treu übersetzen.

**Sicht-Probe LEBT (18.08.): P01-Perkolation im Glasvolumen** —
Film-Sandwich in der Prototyp-Seite (Beauty → additive Punkt-Ebene →
additiver Glas-Deckel), bewiesene Kameraprojektion
(`pruefe-projektion.html`), Songschleife, Bandwelten Amber/Magenta/Eis
(FL-Repo 6b5d4eb). Der User schaut sie an, um seine Vorstellung zu formen.

**Denk-Leinwand steht (18.08.):** `eq-copilot/design/prisma/
prisma-prototyp.html` — drehbares Prisma aus 60 vorgerenderten
Cycles-Frames (klar, 120°-Periode, `renders/dreh/`); Ziehen/Einrasten,
Tasten 1–3, `?winkel=` friert ein. Bewusst ohne Daten-/Effektebene.

Werkzeug steht: `eq-copilot/design/prisma/prisma-material-still.py`
(Blender 5.1 headless, Cycles CPU — nie GPU, Arc-Landmine; CLI: --variante
--samples --breite --hoehe --energie --kamera --ziel --nur-strahl).
Cycles-Grenzen (Spot-Kugelnormierung, Fächer-Sampelbarkeit, MNEE nur eine
Fläche) stehen in `design-stand.md`, Abschnitt AKTIVE RICHTUNG.

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
  (JSON-Backups in `design/unicorn/`). Taste T = Tafel; die Tafel prüft
  seit 19.08. `bindingCount` und prangert definierte-aber-UNGEBUNDENE
  Variablen an. `?hintergrund=<ID>` legt eine zweite Szene als Ebene 0
  dahinter — Vorgabe „Winter Nexus (Remix) 3" `DAGhm8NIQHdX9GhNYmP9`,
  liegt auch live hinter `prisma-schlieren.html` (`?hintergrund=0` für
  Messläufe).
- `eq-copilot/design/proben/` — 4 Kalibrier-Proben + `render-probe.mjs`
  (generisch: Blatt, Name, Query, Größe, Wartezeit; Einzellauf, SwiftShader).
- 30-s-Songschleife (`songLage`/`musikDb`) — in jedem Blatt identisch
  portiert; deterministisch, `?zeit=` friert ein.
- Geparkt, unangetastet: Tiefenfeld + Bauplan 2.0 + Kunstwerk-Studie
  (drei Welten) — nie ungefragt reaktivieren.

## Offene kleine Punkte

- NAK-11: Unicorn-Lizenz („Legend"-Plan für Offline-JSON) vor
  Plugin-Auslieferung klären — Design-Phase unkritisch.
- ~~Nakama-Repo hat kein Git-Remote~~ **Erledigt 19.08.:** privates
  Remote `https://github.com/evenacadia-tech/Nakama` angelegt und
  gepusht (User arbeitet wechselnd Desktop/Laptop). Auf dem Laptop
  einmalig: `git clone`, dann in `eq-copilot/design/vendor-build/`
  `npm ci` + `npx playwright install chromium` (node_modules sind
  bewusst nicht im Repo); Broker per `cargo build --release`,
  Golden-WAVs per `py -3.13 tools/eq-copilot/erzeuge_fixtures.py
  --nur-wav`. Ab jetzt: nach Commits pushen, vor Arbeitsbeginn pullen.
