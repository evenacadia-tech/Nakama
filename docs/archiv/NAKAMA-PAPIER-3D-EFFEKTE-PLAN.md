> **ARCHIV (21.08.2026). Papier-3D-Effekte (E0–E5), vom User 16.08. verworfen („das sieht sehr schlecht aus“). Keine Vorgabe mehr. Gültige Produktdefinition: CLAUDE.md (Wahrheitskern + Entscheide-Register); technischer Entwurf: docs/FL-Nakama-Sonden-Design-Entwurf.md.**

# Nakama — Echte 3D-Papier-Effekte

## Plan für den Umbau der Ereignisse auf dem lebenden Blatt

> **Stand:** 2026-08-16
> **Gilt für:** `eq-copilot/design/nakama-spectral-field-claude.html` (das Claude-Musterblatt)
> **Nicht berührt:** `nakama-spectral-field-vorentwurf.html` (Codex-Besitz; nutzt das
> Vendor-Bundle nicht — Neubau kollidiert nicht)
> **Status:** ⛔ **VERWORFEN (16.08.2026 abends, USER):** „das shader model von
> letzter session wird komplett verworfen — das sieht sehr schlecht aus."
> Die E0–E5-Umsetzung war gebaut und formal bewiesen (Null-Beweis 0,16 %,
> Regler-Beweis 18/18) — der Beweis maß aber nur Pixel-ÄNDERUNG, nie
> Schönheit. three.js-Bühne, Vendor-Bundle, Composer und alle Shader-Effekte
> sind aus dem Musterblatt entfernt (Commit `452e7fd`). Nachfolger:
> **2D-Tusche-Kalligrafie** direkt im Musterblatt — „die Tusche erlebt die
> Befunde": Bruch (Strich zerfasert) · Siegel (Trocknungsringe) · Saugen
> (Sog-Fasern) · Atmen (nur der Strich). Dieses Dokument bleibt als
> Verlaufsbeleg und Warnung stehen: prozedurale Material-Simulation ist an
> dieser Aufgabe zweimal gescheitert.

**User-Auftrag (2026-08-16):** Das Blatt sieht gut aus, die Effekte nicht. Glut und
Riss wirken wie schwebende Schablonen; viele Kalibrier-Regler verändern nichts
Sichtbares. Wir haben ein echtes 3D-Blatt — also echte 3D-Effekte, nicht das
Vorhandene mit anderen Werten.

---

## 1. Befund: warum Glut und Riss wie Schablonen schweben

Aus dem Code gemessen (Zeilennummern: Stand dieses Plans):

1. **Alles ist EINE Albedo-Textur.** Das Material trägt genau `map: textur`
   (Z. 2444–2451); Brand ist ein radialer Farbverlauf, der in den Canvas gemalt
   wird (Z. 2700–2707). Die „Glut" ist damit *Papierfarbe*. Das Licht schattiert
   sie wie Papier: statt zu leuchten wird sie mit-verdunkelt. In einer
   LDR-Pipeline ohne Emissive und ohne Bloom **kann** nichts glühen — das Auge
   erkennt den Widerspruch sofort und liest: Aufkleber.
2. **Der Riss ist eine Stanze mit aufgemaltem Rand.** Alpha-Ausstanzung per
   `destination-out` (Z. 2766) plus dunkler Konturstrich plus kurze Strichlein
   als „Fasern" (Z. 2770–2788). Echtes Papier reißt entlang einer Linie, zeigt
   **helle Rohfaser-Büschel** an der Kante und die Ränder **biegen sich auf**.
   Ein dunkler Umriss auf der Fläche ist exakt das Schablonen-Signal. Dahinter
   liegt nur eine matte Schwarzplatte (Z. 2456–2459) ohne Tiefenwirkung.
3. **Keine Geometrie-Reaktion.** Der Vertex-Loop kennt nur Wind + statische
   Wölbung (Z. 2820–2825). Brand, Riss und Nässe verformen das Blatt nicht —
   das Papier „erlebt" die Befunde nicht, es wird nur bedruckt.
4. **Flaches Licht macht die 3D-Natur unsichtbar.** Ambient 0,62 + ein fast
   frontales Directional 1,05 (Z. 2460–2462): die Wellen erzeugen kaum
   Hell-Dunkel-Modulation auf der Fläche. Nur die Silhouette wackelt — deshalb
   wirkt das Blatt wie ein statischer Hintergrund und der Graph „schwebt".
5. **Mehrere Regler-Senken sind unter der Sichtbarkeitsschwelle.**
   - Papierkorn: Faser-Alpha 0,028–0,078 auf 256er-Kachel (Z. 2514) — unsichtbar.
   - Vorzeichnung: 1,6-px-Linie auf 2048er-Textur ≈ <1 px am Schirm (Z. 2664),
     nur Alpha skaliert.
   - Kasure: zeichnet nur bei `|v| > 85` — seltene Momentspitzen (Z. 2743).
   - Kamera-Nähe: reiner Dolly (Z. 2818) — wirkt wie Zoom, nicht wie Raum.
   - Wölbung: nur Eckbereiche, Amplitude 0,05, und ohne Schattierung ohnehin
     kaum ablesbar (Z. 2478–2482).

**Grundsatz daraus:** Das Blatt ist heute ein *Foto mit wehendem Rand*. Ein
Material wirkt erst echt, wenn ein Ereignis auf **drei Kanälen gleichzeitig**
antwortet:

| Kanal | heißt konkret |
|---|---|
| **Stoff** (per Pixel) | Albedo, Rauheit, Emission, Alpha ändern sich am Ereignisort |
| **Form** (Geometrie) | das Blatt wölbt, rollt, beult sich am Ereignisort |
| **Licht** (Szene) | Glut strahlt aufs Papier; Leuchten blüht über HDR-Bloom |

Genau diese drei Kanäle baut dieser Plan.

---

## 2. Zielarchitektur: Papier · Tusche · Zustand

### 2.1 Drei Texturquellen statt einer

```text
HEUTE   1 Canvas (Papier+Raster+Kurve+Effekte) ──> map ──> fertig

NEU     Tusche-Canvas (transparent):            die ZEICHNUNG
          Raster, Achsen, Kurve, Zonen, Gold    (bleibt 2048×1280, ~30 Hz)
        Papier im Shader:                       der STOFF
          Grundton + Faserkachel + Vignette     (prozedural, Uniform-gesteuert)
        Zustandsfelder (DataTexture 128×80):    das ERLEBEN
          brandFeld · feuchtFeld · rissFeld     (CPU-Update ~15 Hz, R-Kanal)
```

Warum die Trennung zwingend ist: Nässe muss das *Papier* dunkler machen und die
*Tusche* ausbluten lassen — zwei verschiedene Antworten. Brand muss Papier UND
Tusche verkohlen. Die Riss-Kante braucht helle Roh*faser* (Papiereigenschaft).
Mit einer vorkomponierten Textur ist keine dieser Antworten möglich.

### 2.2 Material: Standard-Material erweitern, nicht ersetzen

`MeshStandardMaterial` + `onBeforeCompile` (etabliertes three.js-Muster:
eigene Uniforms + gezielt ersetzte Shader-Chunks). So bleiben Licht, Schatten
und Tonwert-Pipeline erhalten, und wir schreiben nur die Deltas:

- `map_fragment` → Komposition `papier(uv) ⊗ tusche(uv)` + Zustandsbänder
- `roughnessmap_fragment` → nass = glatter, verkohlt = rauer
- `emissivemap_fragment` → Glutsaum als HDR-Emission (Intensität > 1)
- nach `alphatest_fragment` → eigener `discard` für Durchbrand + Risskern
  (`material.alphaTest` fällt auf ~0,01: die heutige 0,35-Schwelle würde
  weiche Fransenpixel hart wegschneiden)
- `gl_FrontFacing` → Rückseite des Papiers einen Hauch heller (sichtbar, wenn
  Risslappen sich aufbiegen)

Alle Uniforms leben in **einem** benannten Block (`NAKAMA_UNIFORMS` im
Quelltext dokumentiert) — das ist zugleich der Portierungsvertrag für die
spätere JUCE/GL-Umsetzung.

### 2.3 Verformung bleibt CPU — bewusste Entscheidung

Der bestehende Vertex-Loop (113×71 ≈ 8 000 Punkte, läuft heute schon bei
60 fps) bekommt drei zusätzliche Feld-Terme:

```text
z = wind(x,y,t) + wölbung(x,y)
  + brandCurl(feldGradient)      Kante rollt sich zur Glut hin auf
  + rissLappen(sdf, seite)       beide Rissufer kippen nach hinten/außen
  + feuchtBeulen(feld × rauschen) Cockling — nasses Papier wirft Beulen
```

Begründung gegen GPU-Displacement: (a) die Marker-Anker müssen derselben
Verformung folgen — künftig durch **bilineares Ablesen des verformten
Gitters** statt der analytischen Windformel; damit bleiben sie automatisch
synchron mit *jeder* Verformungsquelle. (b) `computeVertexNormals()` liefert
korrekte Normalen gratis. (c) Eine Wahrheitsquelle, die 1:1 nach C++
portierbar ist. Feine Details unterhalb der Gitterauflösung (Krakelee an der
Brandkante, Faserstruktur) kommen als **Normal-Störung im Fragment-Shader** —
Schattierungsdetail braucht keine Vertices. Reicht die Gitterauflösung fürs
Lappen-Curling nicht, wird auf 160×100 erhöht (16 k Punkte, weiter unkritisch).

### 2.4 Licht neu + Leucht-Pipeline

- **Licht-Setup:** Key-Directional deutlich schräger (streifendes Licht zeigt
  jede Welle), Ambient runter, dazu ein leises kühles Gegenlicht von rechts
  unten. Erst damit werden Wind, Wölbung und alle Verformungen *ablesbar*.
- **Glut-Licht:** ein `PointLight` am Brandpunkt (Welt-Token `brandKern`,
  Intensität = Brandaktivität × Flackern). Das Papier ringsum wird real warm
  angestrahlt — der stärkste „das gehört zum Blatt"-Beweis.
- **Bloom:** `EffectComposer`-Kette `RenderPass → UnrealBloomPass → OutputPass`
  (verifizierter Stand r180; OutputPass macht am Ende die sRGB-Wandlung).
  `renderer.toneMapping` bleibt `NoToneMapping` — die vier Papier-Welten sind
  farblich kalibriert und dürfen nicht kippen.
- **Selektiv** über den Material-Tausch-Zweitrender (Standardmuster):
  Zweit-Composer rendert das Blatt mit einem Glut-Only-Material (schwarz +
  Emissionsband, gleiche Uniforms), Ergebnis wird additiv über das Basisbild
  gemischt. Nötig, weil in den hellen Welten das beleuchtete Papier selbst
  über jeder Luminanz-Schwelle läge — ein reiner Schwellen-Bloom würde das
  ganze Blatt zum Leuchten bringen.
- **Null-Beweis (Pflicht):** Composer an, alle Ereignisse aus → das Bild muss
  dem heutigen Direkt-Render gleichen (Pixel-Toleranz < 0,5 %). Der
  Pipeline-Umbau darf die Welten nicht verfälschen.

---

## 3. Die vier Ereignisse im Detail

Jedes Ereignis wird auf allen drei Kanälen beschrieben: Stoff → Form → Licht.
Bewegungsverhalten respektiert `prefers-reduced-motion` (Flackern/Pulsieren
aus, Zustände bleiben statisch sichtbar).

### 3.1 Einbrennen (Dauerresonanz) — der Leiteffekt

Technik: klassische Brenn-Rampe („dissolve/burn shader"): Rauschfeld +
wachsende Schwelle, `smoothstep`-Bänder relativ zur Brandfront.

- **Feld:** wächst am Resonanzpunkt mit der Dauer (`brandWert` existiert
  bereits), radial mit Rauschkante — die Front ist unregelmäßig wie echtes
  Glimmen, kein Kreis.
- **Stoff,** vier Bänder von außen nach innen:
  1. *Sengung:* Papier vergilbt → bräunt (multiplikative Tönung), die Tusche
     wird spröde-heller;
  2. *Verkohlung:* fast schwarz, Rauheit ↑, Mikro-Krakelee als Normal-Störung
     (analytisches Rausch-Derivat `noise2d_d` für das Relief);
  3. *Glut:* **„verdientes Licht"** nach dem `magma-crust`-Prinzip aus
     Sonifold — die Glut liegt nie AUF der Kohle, sie scheint durch ein
     **Rissnetz IN der Kohle** (Zellkanten-Distanz F2−F1, exponentieller
     Abfall vom Riss, ein langsam driftendes Wärmefeld entscheidet, WELCHE
     Risse glimmen; Resonanzaktivität = Surge-Verstärkung). Dazu der
     Glutsaum an der Brandfront (dissolve-Kantenzone: fbm-verworfene
     Festigkeit über der Distanz). Beides **HDR-Emission** in
     `brandKern`-Farbe — nur das bloomt;
  4. *Durchbrand:* `discard` — ein echtes Loch mit glimmender Kohlekante.
     Ob es je durchbrennt, ist kalibrierbar (neuer Regler „Brand-Tiefe":
     Glimmen ↔ Durchbrennen).
- **Optionale Zutaten, Entscheid am Bild (User):** feine
  Lichtenberg-Verästelung im Sengsaum (Brand kriecht den Fasern nach,
  ridged-Multifraktal) · eine leise **Chladni-Resonanzfigur** in der
  Sengzone (stehende Wellen — physikalisch das, was Dauerresonanz mit einer
  Platte macht). Beide werden als schaltbare Varianten gebaut, nicht fest
  verdrahtet.
- **Form:** die Papierkante wölbt sich zur Glut hin auf (Curl entlang des
  Feldgradienten) — Hitze arbeitet im Blatt.
- **Licht:** das flackernde PointLight (s. 2.4) + Bloom-Blüte um den Saum.

### 3.2 Riss (Mittenloch)

Formwechsel weg von der Ellipsen-Stanze: **Papier reißt entlang einer Linie.**

- **Geometrie des Risses:** ein gezackter **Risspfad** (8–12 Segmente, fester
  Samen) quer durch die Mittenzone; die Öffnung ist eine Linsenform — Breite
  wächst mit der Aktivität (Spalt ↔ Klaffen, kalibrierbar).
- **Feld:** vorzeichenbehaftete Distanz (SDF) zum Pfad, CPU-seitig ins
  `rissFeld` gerechnet; ein Öffnungsprofil entlang des Pfads steuert, wo der
  Riss klafft und wo er nur angerissen ist.
- **Stoff:** Kern → `discard` (echtes Loch). Fransenband: fasrige
  Alpha-Ausfransung (Rauschen entlang der Pfad-Tangente) und — entscheidend —
  **helle Rohfaser-Kante**, heller als das Papier. Direkt außerhalb eine feine
  AO-Verdunklung (die Kante wirft Mikroschatten). Der Tuschestrich endet
  weiterhin sichtbar VOR dem Loch („die Tusche bricht am Loch") — Pfadpunkte
  sind eine gemeinsame Quelle für Canvas-Zeichnung und Shader-Uniforms.
- **Form:** die beiden Risslappen kippen nach hinten/außen
  (Verformung ∝ e^(−Distanz) × Seite) — durch `DoubleSide` wird die hellere
  Rückseite sichtbar. Der Blick fällt durch eine echte Öffnung.
- **Tiefe dahinter:** die Schwarzplatte bekommt einen Tiefenverlauf statt
  mattem Einheitsschwarz; die vorhandene Parallaxe (z-Abstand 0,3) wirkt erst
  durch die aufgebogenen Lappen.

### 3.3 Vollsaugen (Bass-Stau)

- **Feld:** echte Ausbreitung — Diffusion (Nachbar-Mittelung + Quellterm an
  der Stauzone, 15 Hz) → ein organisch wachsender Nassfleck statt einer
  Kurven-Alpha-Fläche.
- **Stoff:** nass = dunkler und satter (Albedo-Potenz), Rauheit ↓ (feuchter
  Schimmer im Streiflicht), **Trocknungsrand**: die schmale dunkle Kante am
  Feldrand (Kaffeerand — das ikonischste Nass-Papier-Signal). Umsetzung nach
  dem `watercolor-bleed`-Muster aus Sonifold: 5-Tap-Gradient des Feuchtfelds
  → der Ring sitzt genau dort, wo der Feldgradient am steilsten ist UND
  Pigment liegt; die Nässe weitet das smoothstep-Band (nass = weiche Kante,
  trocknend = harte). Die Tusche **blutet aus**: UV-Jitter wo nass, plus das
  `ink-bloom`-Tendril-Muster — ein ridged-Rauschen erodiert den dünnen
  Tuschesaum zu Fäserchen, während der Kern hält.
- **Form:** Cockling — mittelfrequente Beulen im nassen Bereich
  (Rauschen × Feld). Nasses Papier liegt nie plan.

### 3.4 Grundleben (Wellen/Wind)

Bleibt CPU-Wind — wird aber erst durch das neue Licht sichtbar (Schattierung
statt Silhouette). Dazu: Rauheits-Mikrovariation + sehr leichte Faser-Normalen,
damit das Streiflicht über eine lebendige Oberfläche läuft. Der Kamera-Regler
wird vom reinen Dolly zum **flachen Kamerabogen** (Nähe + leichter
Seitenwinkel, lookAt bleibt Blattmitte) — ein Perspektivwechsel beweist die
Räumlichkeit sofort.

---

## 4. Regler: jede Funktion beweist sich sichtbar

### 4.1 Beweis-Harness (neu, Pflicht ab E0)

- `?zeit=<sekunden>` friert die Uhr des Blatts ein → deterministisches Bild.
- `eq-copilot/design/beweis-regler.mjs` (Playwright, ohne neue npm-Pakete):
  lädt das Blatt, stellt jeden Regler nacheinander auf Min und Max, liest die
  Pixel in-page zurück (2D-Canvas-Readback des WebGL-Bilds; ein
  `?beweis=auf`-Parameter schaltet dafür `preserveDrawingBuffer` an — ohne
  ihn ist der WebGL-Puffer nach dem Frame leer) und misst den
  Anteil veränderter Pixel. Ergebnis als Tabelle:
  `design/vorentwurf-renders/regler-beweis.md` (+ `.json`).
- **Akzeptanz:** jeder Regler ≥ 1,5 % veränderte Pixel zwischen Min und Max
  (Schwelle im Skript kalibrierbar). Ein Regler unter der Schwelle ist ein
  FAIL und wird repariert — kein „ist halt subtil".

### 4.2 Bestehende Regler — neue, sichtbare Senken

| Regler | heutige Senke | neue Senke |
|---|---|---|
| Wind-Stärke/-Tempo | Geometrie (unsichtbar bei flachem Licht) | dieselbe Geometrie, sichtbar durchs Streiflicht |
| Blatt-Wölbung | Ecken, Amplitude 0,05 | größerer Bereich + sichtbar über Schattierung |
| Kamera-Nähe | reiner Dolly | Kamerabogen: Nähe + leichter Winkel |
| Pinselbreite | funktioniert | unverändert |
| Trockener Strich | nur bei `\|v\|>85` | Schwelle skaliert mit Regler + permanente leichte Trockenbrüche am Strichrand |
| Vorzeichnung | nur Alpha | Alpha **und** Linienstärke |
| Papierkorn | Alpha 0,03–0,08 | Shader-Uniform Faserstärke, deutlich spürbarer Bereich |
| Saugen/Riss/Brand | 2D-Malstärke | Feld-Verstärkung: Stoff + Form + Licht zugleich |

### 4.3 Neue Regler (Ereignis-Drastik — User entscheidet am Bild)

- **Brand-Tiefe** (Glimmen ↔ Durchbrennen)
- **Glut-Flackern** (ruhig ↔ lebendig)
- **Riss-Öffnung** (Spalt ↔ Klaffen)
- **Licht-Winkel** (frontal-weich ↔ streifend-dramatisch)

Das Kalibrier-Panel bekommt Gruppentitel (**Blatt · Zeichnung · Ereignisse ·
Licht/Kamera**), damit ~15 Regler geordnet bleiben. `localStorage`-Format
bleibt kompatibel: fehlende Schlüssel fallen auf den Standard.

---

## 5. Werkzeuge und Bundle (reproduzierbar)

- **Vendor-Bundle-Neubau:** Das heutige `assets/vendor/nakama-three.js` ist
  three.js-r180-Core + GLTFLoader — **ohne** Postprocessing; ein Build-Skript
  existiert nicht im Repo. Neu: `eq-copilot/design/vendor-build/` mit
  `package.json` (three 0.180.x + esbuild, Versionen gepinnt) und
  `build-nakama-three.mjs`. Gebündelt werden zusätzlich:
  `EffectComposer`, `RenderPass`, `ShaderPass`, `UnrealBloomPass`,
  `OutputPass` (aus `three/addons/postprocessing/…`). Ausgabe bleibt dieselbe
  Datei, Header-Kommentar zählt die Module auf. Kein CDN zur Laufzeit
  (bestehende Regel); das gebaute Bundle bleibt committed, der Bau ist ab
  jetzt reproduzierbar — gleiche Regel wie bei der Nakama-Phase-Schrift.
- **Renders:** weiter `npx playwright screenshot`. Ehrliche Grenze:
  Headless-SwiftShader zeigt Bloom matter und langsamer als die echte GPU —
  Beweisbilder ja, finale Optik-Abnahme macht der User am lebenden Blatt.
- **2D-Fallback** (`?stage=2d`) behält die alten 2D-Ereignisse als Notpfad —
  kein Doppelbau; Abnahme-Maßstab ist die 3D-Bühne.

### 5.1 Transfer-Vertrag — der Web-Aufwand bleibt Eigentum des Plugins

User-Frage (16.08.): „Wäre es nicht sinnvoller, das gleich in die richtige
App zu bauen?" **Entscheidung: Nein — die Design-Phase bleibt im
Musterblatt.** Begründung: (a) die Design-Schleife des Users ist im Browser
Sekunden (Regler ziehen), im Plugin Minuten bis Stunden (Kompilieren +
UAC-Installation + FL-Neustart) — und wir stehen VOR den
Design-Entscheidungen, nicht danach; (b) die produktive Ansicht steht selbst
vor dem Bauplan-2.0-Umbau — 3D in den alten Editor wäre echte
Wegwerf-Arbeit, beide Großbaustellen gleichzeitig hieße lange bauen, ohne
dass der User ein Bild beurteilen kann; (c) eine eigene GL-Pipeline im
DAW-Fenster ist ein bekanntes Risiko-Feld (Kontexte, Treiber, Resize) und
gehört nach den Design-Freeze, nicht in die Suchphase.

Damit der Musterblatt-Aufwand nicht verfällt, sondern überführt wird, gilt
verbindlich:

1. **Effekt-GLSL ist host-neutral.** Alle Effektfunktionen (Rausch/AA-
   Fundament, Papier, Brand, Riss, Feuchte) leben als eigene
   `<script type="x-shader/nakama">`-Textblöcke im Musterblatt: reine
   GLSL-Funktionen ohne three.js-Abhängigkeit. three.js fügt sie nur als
   Text ein (`onBeforeCompile`); JUCE/GL kompiliert später **denselben
   Text** hinter einem dünnen Adapter (`#version`-Kopf, in/out-Mapping).
2. **Auszug statt Hand-Kopie:** beim Start des JUCE-GL-Meilensteins zieht
   `design/extrahiere-shader.mjs` die Blöcke nach
   `eq-copilot/plugin/assets/shader/` — eine Quelle, kein Drift.
3. **Die kalibrierten Zahlen sind der Design-Freeze:** die
   „Werte kopieren"-JSON des Users wird 1:1 zu den JUCE-Standardwerten.
4. **Ehrliche Buchhaltung:** web-only bleiben three.js-Gerüst (~200 Zeilen
   Szenenaufbau), Vendor-Bundle-Bau und der Playwright-Beweis — zusammen
   grob ein Viertel des Aufwands. Doppelt geschrieben wird einzig die
   Tusche-Zeichnung (JS-Canvas ↔ JUCE Graphics) — sie ist in beiden Welten
   die kleinste, klarste Schicht, und ihre JUCE-Seite existiert im
   Plugin-Editor bereits in Grundzügen. Effektformeln, Felder-Logik,
   Sonifold-Ports und kalibrierte Werte — der kreative Kern — wandern
   vollständig.

---

## 6. Sonifold-Ernte — erprobte Bausteine statt Neuableitung

Auf User-Hinweis geprüft (16.08.): `C:\Users\phili\Sonifold-dev` — der über
6 Monate gebaute Musik-Visualizer — enthält eine gereifte
WGSL-Shader-Bibliothek (Compositor-Zutaten mit dokumentierten
Parameterbereichen, gemeinsames Rausch-Fundament, Jimenez-Bloom-Pyramide).
WGSL → GLSL ist mechanische Übersetzung. Regeln der Ernte:

- **Nur Kopie, nie Abhängigkeit:** portierte Funktionen wandern als Kopie in
  das Musterblatt (später in den JUCE-Shader), jeweils mit
  Herkunftskommentar `// Ursprung: Sonifold <pfad>` — keine
  Laufzeit-Verbindung zwischen den Repos, das Musterblatt bleibt eine
  offline lauffähige Datei. Sonifold selbst wird **nicht angefasst**
  (nur gelesen; eigenes Projekt des Users, Wiederverwendung ist sein Recht).
- **Die Trennung der Welten bleibt:** geerntet wird Code, keine Kopplung.

| Sonifold-Quelle | wandert in | was genau |
|---|---|---|
| `crates/sonifold-compositor-core/src/ingredients/common.wgsl` | E1, ein GLSL-Block | das komplette Fundament: `hash21/22`, Gradient-/Value-Rauschen inkl. **analytischem Derivat** `noise2d_d`, fbm-Familie (auch ridged/eroded), SDF-Helfer — und die **Anti-Aliasing-Familie** (`aa_step`, `aa_line_width`, `grain_aa_gain`): Korn und dünne Linien flimmern damit nicht unter der Blattbewegung (dieses Problem hatte der Plan bislang gar nicht adressiert) |
| `ingredients/magma-crust.wgsl` | E2 Brand | das Glut-Prinzip „verdientes Licht": Rissnetz aus Zellkanten (F2−F1), exponentieller Glutabfall im Riss, Wärmefeld-Gate (nur manche Risse glimmen), Surge-Anhebung durch Aktivität, „Korn multipliziert, addiert nie" |
| `ingredients/dissolve.wgsl` | E2 Brandfront + E3 Riss-Fransen | Kantenzone als fbm-verworfene Festigkeit über der Distanz (Auflösung IST die Kante — kein harter Rand), Kohärenz-Boden gegen Zerfall |
| `ingredients/lichtenberg-scar.wgsl` | E2, optionale Zutat | ridged-Multifraktal-Verästelung (Brand kriecht Fasern nach), radiale Verjüngung, helle Spitzen |
| `ingredients/chladni-plate.wgsl` | E2, optionale Zutat | Resonanzfigur (stehende Wellen) leise in der Sengzone — Entscheid am Bild |
| `ingredients/watercolor-bleed.wgsl` | E4 Feuchte | Watermark-Ring über verschmolzene 5-Tap-Feldgradient-Abtastung, Nässe weitet die Kanten-Bandbreite, Papier-Makro/Mikro-Korn-Schichtung |
| `ingredients/ink-bloom.wgsl` | E4 Ausbluten | Tendril-Erosion: ridged-Multiplikator frisst den dünnen Saum zu Fäserchen, der Kern hält |
| `shaders/bloom_down.wgsl` + `bloom_up.wgsl` | Eskalationsstufe zu E2 | **Jimenez-13-Tap-Bloom-Pyramide** (Karis-Mittelung gegen Glitzerpunkte, ringing-frei — Industriestandard). E0 startet mit three.js `UnrealBloomPass`; zeigt der an dünnen Glutlinien Ringing/Fireflies, wird auf den portierten Jimenez-Bloom gewechselt (beides Composer-Pässe, Wechsel ist lokal). Für die spätere JUCE/GL-Umsetzung ist der Jimenez-Port ohnehin der gesetzte Weg — dort gibt es kein three.js. |

Mitgeerntete Arbeitsregeln aus Sonifold (bewährt, gelten ab jetzt auch hier):
**Korn multipliziert, addiert nie** (sonst wird Dunkles gräulich) ·
**Licht ist verdient** (jedes helle Pixel hat eine physische Begründung —
Glut ist Blick ins Innere, kein Overlay) · Zutaten-Stil: jede Effektfunktion
ist eine pure Funktion mit dokumentierten Parameterbereichen (das ist zugleich
der JUCE-Portierungsvertrag). Und die Maschinen-Landmine gilt weiter:
**keine GPU-Batch-Render-Loops** auf der Arc A770 — die Beweis-Renders laufen
in Software (SwiftShader) und bleiben Einzelläufe.

---

## 7. Meilensteine und Prüfungen

Reihenfolge ist verbindlich; jeder Meilenstein endet mit Commit + Beweis.
Zeiten sind AI-realistische Arbeitsminuten.

| # | Inhalt | Prüfung | ~Zeit |
|---|---|---|---|
| **E0 Fundament** | Bundle-Neubau (+Addons, Build-Skript), Composer-Kette, `?zeit`-Freeze, Beweis-Skript-Gerüst | `node --check`; **Null-Beweis:** Composer an / Ereignisse aus → Pixel-Diff < 0,5 % gegen Direkt-Render; 4 Welten-Renders unverändert | 30 min |
| **E1 Blatt & Licht** | GLSL-Fundament-Port (`common.wgsl`: Rauschen + AA-Familie), Papier/Tusche-Trennung im Shader, neues Licht-Setup, Faser-Uniform + Faser-Normalen (flimmerfrei via `grain_aa_gain`), Kamerabogen | Beweis-Lauf Wind/Wölbung/Korn/Kamera ≥ Schwelle; Welten-Abnahme-Renders | 45 min |
| **E2 Brand** | brandFeld, 4 Bänder, **Glutadern durchs Kohle-Rissnetz** (magma-crust-Port) + dissolve-Front, HDR + selektives Bloom, PointLight, Curl; Regler Brand-Stärke/-Tiefe/Flackern; optionale Zutaten (Lichtenberg/Chladni) schaltbar | Render-Serie Brand 0/50/100/Durchbrand je Welt; Bloom-Qualität an dünnen Glutlinien prüfen (sonst Jimenez-Eskalation, §6); Beweis-Lauf | 75 min |
| **E3 Riss** | Risspfad + SDF-Feld, Fransen + helle Rohfaser-Kante, Lappen-Curl, Tiefe dahinter; Regler Riss-Größe/-Öffnung | Render-Serie; Sichtprüfung Marker-Anker folgen dem verformten Gitter | 60 min |
| **E4 Feuchte** | Diffusionsfeld, Nass-Look + Trocknungsrand (watercolor-bleed-Port), Cockling, Tusche-Ausbluten (ink-bloom-Tendrils) | Zeitserien-Renders (Fleck wächst organisch); Beweis-Lauf | 45 min |
| **E5 Generalprobe** | Beweis-Tabelle ALLE Regler grün; Galerie 4 Welten × 4 Ereignisse + Vorher/Nachher; Doku-Nachzug (CLAUDE.md, dieser Plan → Status) | vollständiger `beweis-regler`-Lauf ohne FAIL; Galerie liegt in `vorentwurf-renders/` | 30 min |

**Performance-Budget:** Felder 128×80 @ 15 Hz (CPU trivial), Tusche-Canvas
bleibt ~30 Hz, Vertex-Loop ≤ 16 k Punkte, Bloom rendert intern herunterskaliert;
Ziel bleibt flüssige 60 fps auf dem Zielrechner (Arc A770).

---

## 8. Risiken und ehrliche Grenzen

- **Schatten:** Das Blatt wirft/empfängt derzeit keine Schatten. Falls später
  Selbstschatten gewünscht: eigenes Depth-Material nötig, das die
  `discard`-Löcher repliziert — bewusst NICHT in diesem Plan.
- **SwiftShader-Beweisbilder** unterschätzen Bloom — User-Abnahme am lebenden
  Blatt ist Teil von E5.
- **Farb-Drift durch Composer:** durch den Null-Beweis in E0 abgeriegelt.
- **Marker-Anker** verschieben sich mit, wenn Lappen/Curl das Gitter verformen
  — gewollt (sie sitzen AUF dem Blatt), wird in E3 gezielt geprüft.
- **JUCE-Portierung** bleibt eigener Meilenstein nach Design-Freeze
  (Bauplan-Status): dieser Plan hält dafür alle Formeln in einem
  dokumentierten Uniform-Block zusammen.

---

## 9. Technik-Verifikation (Quellen, geprüft 2026-08-16)

- three.js-Doku: [EffectComposer](https://threejs.org/docs/pages/EffectComposer.html) ·
  [UnrealBloomPass](https://threejs.org/docs/pages/UnrealBloomPass.html)
  (Addons-Pfade; Kette RenderPass → Bloom → OutputPass; OutputPass wandelt am
  Ende in sRGB)
- [Wael Yasmina — Unreal Bloom Selective](https://waelyasmina.net/articles/unreal-bloom-selective-threejs-post-processing/)
  · [Sangil Lee — Selective Bloom](https://sangillee.com/2025-01-28-selective-bloom-effect/)
  (Material-Schwärzung + Zweit-Composer + additive Mischung)
- [Dusan Bosnjak — Extending three.js materials with GLSL](https://medium.com/@pailhead011/extending-three-js-materials-with-glsl-78ea7bbb9270)
  · [Codrops — Animated Displaced Sphere](https://tympanus.net/codrops/2024/07/09/creating-an-animated-displaced-sphere-with-a-custom-three-js-material/)
  (`onBeforeCompile`-Muster: Uniforms + Chunk-Ersatz, Licht bleibt erhalten)
- [Kyle Halladay — A Burning Paper Shader](https://kylehalladay.com/blog/tutorial/2015/11/10/Dissolve-Shader-Redux.html)
  · [GameDevBill — Paper Burn Shader](https://gamedevbill.com/paper-burn-shader-in-unity/)
  · [Daniel Ilett — Dissolve Effect](https://danielilett.com/2020-04-15-tut5-4-urp-dissolve/)
  (Rausch-Schwelle, smoothstep-Bänder, verkohlte Kante, HDR-Glutsaum + Bloom)
