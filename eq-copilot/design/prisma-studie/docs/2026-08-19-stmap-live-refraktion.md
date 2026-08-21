> **PRISMA-STUDIE — User-Idee, geparkt (User 21.08.2026: „Familie; Prisma nur Studie“ · „Alles nur Studie“ · Herkunft: „Meine Idee“). Kein Produktteil, kein Bauplan, kein Vokabular für die Plugin-UI. Statusblatt: ../STATUS.md. Pfade in diesem Dokument meinen den Stand VOR dem Umzug (eq-copilot/design/prisma/… → eq-copilot/design/prisma-studie/prisma/…; docs/design-stand.md u. a. → prisma-studie/docs/).**

# ST-Map-Live-Refraktion — Recherche-Report (NAK-16)

> 2026-08-19 · /deep-research (5 parallele Worker, 148 Quellenzugriffe,
> Blender-Aussagen zusätzlich am installierten 5.1.2 verifiziert).
> Zweck: Der NAK-16-Bauplan soll auf Film-VFX-/Game-Fachwissen stehen,
> nicht auf Erste-Eingebung — „blender kostet viel zeit, das muss
> perfekt sein" (User, 19.08.).

## TL;DR

Der Ansatz (Refraktion backen, WebGL bricht den lebenden Hintergrund)
ist etablierte Praxis mit dokumentiertem Präzedenzfall. Aber der
ursprüngliche Bauplan hätte an **fünf tragenden Stellen** Blender-Zeit
verbrannt; die Recherche ersetzt ihn durch die Pipeline unten. Kern:

1. **Refraction BSDF, nicht Glass BSDF** — Glass mischt Fresnel-Reflexion
   in die Karte (Kontamination); Refraction ist der reine Baustein.
2. **32-bit-Float-EXR, nicht 8-bit-PNG-Hi/Lo** — der Film-Standard für
   Distortion-Karten ist zwingend 32-bit float; der Browser-PNG-Pfad
   zerstört 16-bit-Daten, und Hi/Lo-Split bricht unter bilinearer
   Filterung.
3. **Karte filterlos rendern (Box 0,01), Denoise aus** — gemittelte
   UV-Werte an Silhouetten sind Kantenmüll; Glättung gehört in den
   Sampler, nicht in die Karte.
4. **Offset-Form statt Absolut-Form für die Laufzeit** — Offsets clustern
   um 0, dort reicht Half-Float; als RG16F-Textur ist LINEAR-Filterung
   WebGL2-Kern (kein Extension-Risiko).
5. **Composite-Formel ist Lerp, nicht Addieren:**
   `(Tint × gebrochener Hintergrund) × (1−Fresnel) + Glanz × Fresnel` —
   und die Karte liefert das Transmissionsgewicht gleich mit
   (R/B-Normalisierung, s. u.).

## Die gehärtete Pipeline (Entscheidungen für NAK-16)

### A. Backen (Cycles 5.1.2, pro Drehframe)

**Szene:** Plate-Fläche bekommt statt des Nexus-Bilds ein reines
Emission-Material mit **prozeduralem ST-Gradienten** (aus Texture
Coordinate/UV — prozedural, damit keine Bildtextur-Farbraumfalle greift;
falls doch Bild: zwingend Non-Color [Blender-Manual]). Kanäle:
**R = u, G = v, B = 1** (Gültigkeits-/Gewichtskanal — Blenders eigener
UV-Pass nutzt exakt diese Konvention, B konstant 1 [bundled manual,
passes]). Boden und alle Lichter aus; nur Plate emittiert.

**Prisma:** **Refraction BSDF**, Color = Weiß, Roughness = 0,
IOR = 1,474 (Mitte, EIN IOR — Dispersion s. u.).
Begründung: „The Glass BSDF is used to add a Glass-like shader mixing
refraction and reflection at grazing angles" — Reflexion würde die Karte
kontaminieren. Der Refraction BSDF ist laut Doku der reine Baustein
(„should be considered as a building block … mixed with a glossy node
using a Fresnel factor") [bundled manual, refraction.rst/glass.rst].

**TIR-Politik:** Bei Totalreflexion liefert Refraction BSDF Schwarz —
„there's literally no possible valid refracted direction"
[projects.blender.org #103491]. Diese Pixel sind über **B ≈ 0** als
ungültig erkennbar; die Laufzeit zeigt dort nur den Glanz-Pass
(Spiegel-Look — physikalisch ehrlich: TIR-Zonen zeigen keinen
Hintergrund).

**Render-Einstellungen (am installierten 5.1.2 verifiziert):**
- `film_transparent = True` → Fehlschüsse (Strahl verfehlt Plate) sind
  Alpha 0 [bundled manual, film.rst].
- `use_denoising = False` — Default ist True (gemessen); OIDN ist ein
  ML-Filter und verschmiert Koordinatendaten [bundled manual,
  denoise.rst].
- `pixel_filter_type = BOX`, `filter_width = 0.01` (Minimum; exakt 0
  gibt es nicht — gemessen). Box = „No filter" [bundled manual,
  film.rst]. Film-Praxis: „If we use UV maps, we make sure there is no
  filter when rendered, or you will get junk on the edges"
  [nuke-users-Archiv]; für Blender identisch dokumentiert
  [blenderartists: „switch the film filter size to 0"].
- Samples: Roughness 0 = Delta-Pfad, deterministisch; Rauschquelle ist
  nur Subpixel-Jitter → **~32–64 Samples genügen** (eigene Ableitung des
  Workers; am Probe-Frame messen). Adaptive Sampling für Determinismus
  aus.
- Transmission Bounces: Default 12 reicht für 2 Grenzflächen + interne
  Reflexe [bundled manual, light_paths.rst].
- **Output: OpenEXR, Full Float (32 bit), RGBA.** EXR speichert
  szenenlinear und umgeht den View-Transform — AgX greift nur auf
  Display-Formate wie PNG [bundled manual, image_formats.rst +
  color_management]. Film-Standard: „Distortion maps must be 32 bit
  floats … at 16-bit … it nearly always causes artifacts"
  [nuke-users-Archiv, benmcewan.com]; OpenEXR reserviert FLOAT explizit
  für Datenkanäle [openexr.com].

**R/B-Normalisierung [eigene Ableitung, mathematisch zwingend]:** Der
Renderer liefert pro Pixel Σ throughput·Emission. Falls der Refraction-
Pfad ein Energiegewicht trägt (Fresnel-Durchlass), sind R und G damit
skaliert — aber B (=1 emittiert) exakt gleich. **u = R/B, v = G/B**
entfernt jede Gewichtung (das ist wörtlich „unpremultiply", die
Premult-Falle der Film-Praxis: „Check this if the UV … channels have
been premultiplied" [Foundry STMap-Doku]); **B selbst IST das
Transmissionsgewicht** für den Composite. Ein Render liefert also Karte
UND Gewicht. Ob Cycles' Refraction BSDF überhaupt Fresnel-gewichtet
(Doku legt nahe: nein, das Mischen ist Nutzersache), stellt die Probe
fest — die Division ist in beiden Fällen korrekt.

**Weiche Kanten:** Die Karte bleibt hart (filterlos). Die weiche
Silhouette fürs Compositing kommt aus dem **normal gefilterten
Glanz-Pass** (existiert: `--nur-glas`) — Praxis-Muster „Objekte getrennt
remappen und danach compen" [nuke-users-Archiv].

**Dispersion:** EINE Karte mit mittlerem IOR; Chromatik zur Laufzeit
über **3 Abgriffe entlang des Offset-Vektors** (R/G/B verschieden
skaliert) — Standardtechnik in Echtzeit [maximeheckel.com, gmshaders.com].
Eskalationspfad (falls die Probe zu wenig Farbsaum zeigt): drei Karten
mit den drei Kanal-IORs (dann 3× Renderzeit).

### B. Konvertierung (offline, Node)

EXR (32f) → Offsets rechnen: `off = uv_karte − uv_identität` (Identität
= (x+0,5)/Breite — der Halbpixel zentriert auf Pixelmitten
[benmcewan.com]) → als **Float16/Float32-Puffer** speichern (eigenes
Binärformat oder 16-bit-kodiert; NICHT durch den Browser-Bildpfad — der
konvertiert PNGs auf 8 bit und wendet ggf. Gamma/ICC an [pngtoy-README];
UPNG.js könnte 16-bit-PNG lesen, liefert aber big-endian-Rohdaten
[UPNG-README] — ein eigenes Binärformat ist einfacher und exakt).

**Präzisions-Mathe der Offset-Form [eigene Ableitung]:** Half-Float hat
10 Mantissenbits. Absolute UV nahe 1,0 quantisieren auf 2⁻¹¹ ≈ 0,37 px
bei 768 px — genau das Film-Argument gegen 16 bit. Offsets liegen aber
nahe 0 (typisch |off| < 0,1, Maximal ~0,3): bei 0,1 ist der
Half-Schritt ~6·10⁻⁵ ≈ 0,05 px, bei 0,3 ~0,19 px. Damit ist die
Offset-Form in 16F tragfähig; die Absolut-Form wäre es nicht. Beide
Formen sind verlustfrei ineinander umrechenbar [Foundry
STMap/IDistort-Doku, erwanleroy.com].

### C. Laufzeit (WebGL2 im Blatt)

- **Karten-Textur: RG16F** (Offsets), Upload aus konvertierten Daten.
  LINEAR-Filterung von Half-Float ist **WebGL2-Kern** („In WebGL2, the
  functionality of this extension is available … by default" [MDN,
  OES_texture_half_float_linear]). 32F bräuchte OES_texture_float_linear
  auch in WebGL2 [MDN] — als Eskalationspfad: RG32F + NEAREST + eigene
  bilineare Filterung im Shader (4 Abgriffe; etabliertes Muster
  [GPU Gems 2 Kap. 27]).
- **Gewichts-/Gültigkeitskanal (B)** als zweite Textur oder dritter
  Kanal (RGBA16F): steuert Mischung und maskiert TIR.
- **Live-Canvas:** pro Frame `texImage2D(unicornCanvas)` in
  vorallozierte Textur (`texSubImage2D`), Uploads VOR den Draws bündeln
  („Prefer doing uploads before starting drawing" [MDN Best Practices]);
  `UNPACK_PREMULTIPLY_ALPHA_WEBGL` bewusst setzen (2D-/WebGL-Canvas sind
  intern premultiplied [webglfundamentals]). Chromium nimmt den
  GPU-GPU-Pfad (historische Messwerte: Chrome 27 ms vs. Firefox 3758 ms
  für denselben Upload [Bugzilla #1246410]) — 1–2 MP @ 60 fps auf
  Desktop realistisch, **im Probe-Still per Mikro-Benchmark messen**
  (die Zahlen sind von 2016).
- **Shader:** `bg_uv = frag_uv + offset` (+ 3-Tap-Chromatik);
  `farbe = tint · bg(bg_uv) · gewichtB` — darüber der Glanz-Pass.
  Kanonische Form: „return vDiffuse * vFinal * (1 - fresnel) + vEnvMap *
  fresnel" [GPU Gems 2 Kap. 19] — Fresnel steckt bei uns gebacken in
  Gewicht (1−F) und Glanz-Pass (F), nichts wird doppelt gezählt.
- Alpha: kein `alpha:false` (Performance-Falle [MDN Best Practices]);
  volle Deckung als Alpha=1 schreiben, Transparenz über die
  Glanz-Pass-Silhouette.

### D. Gültigkeitsgrenzen (bewusst akzeptiert)

Die Karte gilt für **eine** Hintergrund-Ebene in gebackener Tiefe und
die gebackene Kamera — exakt unsere Lage (Nexus = vollflächige Ebene,
Kamera fix pro Frame). Die Ein-Ebenen-Näherung ist in der Praxis
explizit akzeptiert („Although this result is inaccurate … it works well
in practice" [GPU Gems 2]); die Zwei-Grenzflächen-Echtzeitforschung hat
dieselbe Restriktion („only refract infinite environment maps" [Wyman
2005]). Gerade hier schlägt die gebackene pfadgetracte Karte die
Screen-Space-Näherung der Engines: zwei Grenzflächen, TIR, Bündelung
sind IN der Karte, während Engines mit Ein-Interface-Offsets arbeiten
[GPU Gems 2, Unreal-Doku, froyok.fr].

## Was der ursprüngliche Plan falsch hatte (Delta)

| Alt (Erste Eingebung) | Neu (belegt) |
|---|---|
| 8-bit-PNG mit Hi/Lo-Byte-Split | 32f-EXR → Float-Puffer; Hi/Lo bricht unter bilinearer Filterung (unabhängige Kanalinterpolation), Browser-PNG-Pfad zerstört 16-bit-Daten |
| „16 Bit Präzision nötig" (absolut) | Absolut braucht 32 bit (Film-Konsens); erst die OFFSET-Form macht 16F tragfähig |
| Glass BSDF mit einem IOR | Refraction BSDF — Glass kontaminiert die Karte mit Fresnel-Reflexion |
| Filter unerwähnt | Box + 0,01 px Pflicht; sonst Kantenmüll beim Remap |
| Kanten/Coverage unerwähnt | Weiche Kante aus dem gefilterten Glanz-Pass, Karte bleibt hart |
| Gewicht/TIR unerwähnt | B-Kanal = Gewicht + TIR-Maske; R/B-Normalisierung gegen Premult-Falle |

## Konfidenz & offene Punkte

**Hoch (primär + trianguliert):** Refraction-BSDF-Semantik, EXR/AgX-
Verhalten, Filter-/Denoise-Regeln, 32-bit-Film-Standard, RG16F-LINEAR
als WebGL2-Kern, Composite-Formel, Ein-Ebenen-Akzeptanz.

**Mittel (einzelbelegt oder abgeleitet, Probe misst):**
- TIR-Schwarz-Verhalten (offizieller Tracker, eine Quelle) und ob
  Refraction BSDF Fresnel-gewichtet → **Probe: B-Kanal-Werte ansehen.**
- Sample-Zahl 32–64 (Ableitung) → Probe misst Rauschen.
- Canvas-Upload-Kosten heute (Messwerte von 2016) → Mikro-Benchmark.
- RG16F-Upload direkt aus Float32Array (Treiberkonvertierung) → zur
  Sicherheit clientseitig nach Float16 wandeln oder live testen.
- Bower-Artikel (exakter Präzedenzfall, „refraction reference map's red
  and green channels exactly represent UV coordinates … don't require
  any further calculation by the GPU") war nur per Suchauszug lesbar
  (403) — Einzelquelle, aber durch GPU-Gems-/DuDv-Praxis abgestützt.

**Bewusste Nicht-Ziele:** echte Mehrschicht-Lookups (eine Karte = ein
Quellort pro Pixel; TIR-Mischzonen zeigen Glanz statt Hintergrund);
Dispersion als 3-Karten-Präzision (erst wenn 3-Tap zu wenig ist).

## Quellen (gelesen, sofern nicht anders markiert)

Primär: learn.foundry.com (STMap, IDistort) · openexr.com (Technical
Introduction) · bundled Blender-5.1-Manual (refraction, glass, film,
passes, image_formats, color_management, light_paths, denoise) ·
projects.blender.org #103491 · developer.nvidia.com GPU Gems 2 Kap. 19 +
27 · cwyman.org (Graphite 2005) · MDN (OES_texture_half_float_linear,
OES_texture_float_linear, EXT_texture_norm16, WebGL Best Practices) ·
docs.blender.org (passes) · dev.epicgames.com (Refraction) ·
bugzilla.mozilla.org #1246410 · github.com/photopea/UPNG.js.
Sekundär: benmcewan.com · nukepedia.com · erwanleroy.com ·
nuke-users-Mailarchiv · blenderartists.org · webgl2fundamentals.org ·
webglfundamentals.org · froyok.fr · maximeheckel.com · gmshaders.com ·
keheka.com · novedge.com · compositingmentor.com · scratchapixel.com ·
chinedufn.com · lettier.github.io · offscreencanvas.com.
Nur Suchauszug (nicht Volltext): beclamide.medium.com (Bower) ·
gamedev.net (Hi/Lo-Filterproblem) · pngtoy · Unity-HDRP-Doku.

## Methodik

5 parallele Worker (Film-STMap · Blender-Baking · Game-Echtzeit ·
WebGL2 · Glas-Composite), je Objective/Format/Quellen/Boundary;
search-then-fetch, Primärquellen bevorzugt; Blender-Behauptungen
zusätzlich headless am installierten 5.1.2 gemessen (filter_width-Range,
Denoise-Default, Bounce-Defaults). Tragende Claims mehrquellig; Einzel-
quellen und eigene Ableitungen sind im Text markiert. Kein Claim ohne
gelesene Quelle; 403-gesperrte Seiten sind als Suchauszug deklariert.
