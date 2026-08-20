# NEXT-SESSION — Einstieg für die nächste Runde

> **WORKSPACE-UMZUG 18.08.2026:** Alles lebt jetzt HIER
> (`Projekte\Nakama`) — eq-copilot/, broker/ (eigenständig,
> eqcop-broker.exe), tools/. FL-Repo-SHAs in älteren Einträgen unten
> gehören zur FL-Studio-Historie (bis `7964777`).

> Stand: 2026-08-19 spätabends. Diese Datei ist der Schnellstart;
> Tiefe in `docs/design-stand.md` und `docs/geschmacksprofil.md`.
> **Einstieg nächste Session (User-Ansage 20.08.): ZUERST `/freshen`**
> (Memory-/CLAUDE.md-Hygiene), dann hier weiter.

## Visueller Nordstern — vor jeder Designarbeit lesen

Der User hat am 19.08. das langfristige Zielbild **Nakama als
Hörkompass** freigegeben. Verbindlicher Zielvertrag und
Fünf-Keyframe-Storyboard:
`docs/visuelles-zielbild-hoerkompass.md`.

Kurzform: Der Mix bleibt unsichtbar; nur dort, wo er sich selbst im Weg
steht, bricht das klare Prisma das Licht. Gesund = leeres Glas. Nur
belastbare Befunde erzeugen eine lokale Schlieren-Messung. Das Prisma
wendet die betroffene Fläche zum User; Klick öffnet Ursache, Hörbeweis und
genau einen nächsten Versuch; nach erfolgreicher Prüfung klärt sich das
Glas wieder. **Keine neue Grundmetapher und kein Dauer-Visualizer.**

Dieser strategische Lock ersetzt NICHT den unmittelbar freigegebenen
Technikschritt unten. NAK-16 muss dem Zielbild dienen, darf es aber nicht
durch einen neuen Effekt oder eine neue Metapher umdeuten.

## Der eine nächste Schritt

**NAK-16 PROBE-STILL IST GEBAUT UND BESTANDEN (19.08., Commits cba6bd6 +
602b20a). Nächster Schritt: der User SIEHT SICH DAS LEBENDE BLATT AN —
`eq-copilot/design/prisma/stmap-probe.html` doppelklicken (Testmuster)
und mit `?bg=unicorn` (lebender Nexus, braucht Internet).** Optik-Urteil
gehört ihm: Dispersion-Stärke (`?dispersion=0.035` Vorgabe), Glanz-Rig
(zwei Kanten + Front-Streifen, Dark-Field), TIR-Zonen (12 % der
Glasfläche zeigen ehrlich nur Glanz statt Hintergrund).

Was bewiesen ist (`renders/stmap/messung.json` + probe-live-a/b.png +
probe-live-unicorn.png): **74,2 % der Glaspixel ändern sich mit dem
Hintergrund-Stand** (Erfolgsmaßstab der Freigabe — Ähnlichkeit zum alten
Frame bleibt als Metrik VERBOTEN); Konsistenz außerhalb 0/255;
Direktsicht-Beweis p99 0,019 px; B nicht Fresnel-gewichtet; 64 Samples
genügen; Half-Float trägt; RGBA16F-Upload ok. Fallen + Details:
`docs/design-stand.md` (NAK-16-Abschnitt); verbindliche Pipeline
weiterhin `docs/research/2026-08-19-stmap-live-refraktion.md`.

**Lichtwelt-Workflow steht (19.08. nachmittags):** Der User designt die
Ebene 0 SELBST in Unicorn („Fläche mit Raumtiefe, Licht von weit her");
Orientierung: `docs/spielregeln-skizze.html` (4 Tafeln, auch als
privates Artifact) + `prisma/vorlage-lichtwelt.html` (Referenzbild) +
`prisma/vorlage-tiefenkarte.html` (Parallax-Depth-Map, ?horizont=).
Das Probe-Blatt hat ein **Lade-Panel** (unten rechts): Projekt-ID oder
Unicorn-JSON-Export direkt laden (Blob-URL, kein Publish nötig) —
Technik aus dem evenacadia-site-Editor übernommen, SDK v2.2.10 lokal
gevendort (`design/vendor/`), Mount/Destroy serialisiert; alle drei
Wege headless bewiesen. Front-Kicker aus dem Glanz-Pass ist RAUS
(User-Befund: statisches Band = eingebackene fremde Beleuchtung).
Sonifold-Ingredients (WGSL) portiere ich bei Bedarf pro Effekt nach
GLSL.

**72er-SEQUENZ GEBAUT (19.08. abends): das Prisma DREHT im Probe-Blatt.**
Im Panel „DREHKARTEN LADEN …" →
`renders/stmap/dreh-karten.bin.gz` wählen (17,4 MB: 72 ST-Maps halbe
Auflösung + Silhouetten-Crop + Glanz-WebPs, gzip); danach Ziel „Prisma"
→ Ziehen dreht (Trägheit + Einrasten auf 58,3°), Mausrad schubst.
Glanz-Rig wird PRO FRAME am evaluierten Mesh gerechnet (Silhouetten-
Ecken + Weißbrand-Riegel — das mitdrehende Rig war falsch, die
Spiegelbedingung braucht die feste Kamera). **Gemessene Eigenschaft,
kein Bug:** in den Edge-on-Zonen (±30° um die Kanten-Pose, 3×/Umdrehung)
ist der TIR-Anteil 99 % → das Glas verdunkelt beim Wenden und klart an
den Rastposen (12 %) auf — ehrliche Physik ohne Umgebungs-Spiegelung;
ob das als Geste trägt, ist User-Urteil (Eskalation wäre eine
Reflexions-Karte). Regenerieren: `--nur-stmap --dreh 72` (5 min) +
`--nur-glanz --dreh 72` (13 min) + `konvertiere-stmap.py --dreh 72`;
EXRs sind gitignored, Paket + Glanz-WebPs versioniert.

**Danach (braucht ggf. frischen Freigabe-Marker):**
Integration ins Prototyp-/Schlieren-Blatt: Ebene 0 braucht ÜBERSTAND
(Ziel-UVs bis u=1,5; Probe nutzt 1920×1280 hinter 768×1024-Glas),
getContext-Patch (preserveDrawingBuffer) VOR dem Unicorn-SDK-Load
mitnehmen; Offsets sind Glas-Canvas-UV → affin in Hintergrund-UV.

Maschinen-Fakten Sequenz: Kamera = `KAM` im Blatt (lens 65,
−1,65/−3,53/0,95 → Ziel 0/0/0,8 — Code im Blatt schlägt ältere
Notizen), 768×1024; `--weiter` überspringt existierende Frames; alte
Beauty-Frames: ~75 s/Frame bei 320 Samples.

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

**KREATIV-SCHLEUSE:** Der aktuelle Marker `.claude/kreativ-freigabe.md`
enthält getrennt (a) den engen NAK-16-Bau-Scope und (b) den strategischen
Hörkompass-Lock. Der Nordstern ist keine pauschale Baufreigabe; nach Ablauf
der 24 h oder für zusätzlichen Scope erneut mit User-Wortlaut schreiben.

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

## Historischer Stand vor dem Hörkompass-Lock (P01 im Glas — verworfen)

Dieser Abschnitt ist nur Verlaufsbeleg. Die damalige Suche nach einem
Flächen-Inhalt ist durch `docs/visuelles-zielbild-hoerkompass.md` beendet.
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
Selektion, kompakter Dauerbegleiter statt Analyzer-Fenster. Gesund =
leeres Glas; nur belastbare Befunde brechen lokal das Prüffeld. Klick auf
einen Befund öffnet Ursache und Hörbeweis. Der Newton-Bündelmodus bleibt
als möglicher Gesamtblick erhalten, seine Geste ist noch offen.
Darstellung: lebender Hintergrund → ST-Map-Live-Refraktion → lokale
Schlieren-Befundebene → Glanz/Kanten. Details: `design-stand.md` und
`docs/visuelles-zielbild-hoerkompass.md`.

## Nicht verhandelbar (aus dieser Session gelernt)

- **KREATIV-SCHLEUSE:** Nie wieder losbauen ohne mit dem User
  ausgearbeitete Idee. Hook blockt Write/Edit unter `eq-copilot/design/`
  ohne frische `.claude/kreativ-freigabe.md` (Marker gilt 24 h — für die
  nächste Session neu schreiben, mit dem, was der User freigibt!).
- **Geschmacksprofil lesen** (`docs/geschmacksprofil.md`) vor jeder
  visuellen Entscheidung: die drei Gesetze + 3a–3c + Meta-Erkenntnis
  (Vision kommt vom User; Claudes freie Bilderfindung = Mode-Collapse;
  P01-Punktästhetik ist als Ästhetik abgenommen, als rohe Dauerwolke aber
  ausdrücklich verworfen).

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
