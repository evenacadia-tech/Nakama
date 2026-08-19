# Design-Stand — an welchem Prototypen wir arbeiten

> **Stand: 2026-08-19.** Lebendes Dokument — nach jedem Design-Commit
> hier nachziehen (seit dem Workspace-Umzug 18.08. liegt alles in diesem
> Repo). Bei Widerspruch gewinnt der Code (`eq-copilot/design/…`) und der
> Bauplan dort.

## AKTIVE RICHTUNG (Stand 17.08. abends): DAS PRISMA

Nach der Proben-Kalibrierung (unten) hat der **User selbst** das Konzept
autorisiert — und es ist das tragfähigste des Projekts:

**Ein dreiseitiges, drehbares Prisma als kompakter Dauerbegleiter.**
Drei Flächen = drei Blickrichtungen (Tiefen / Mitten / Höhen); Drehen ist
SELEKTION („was will ich gerade sehen"), kein Deko. Der physikalische
Bündelgedanke bleibt als möglicher Gesamtblick erhalten (Newton: zweites
Prisma bündelt Spektrum zu Weiß — mathematisch die Inverse der Analyse),
aber seine Geste ist noch offen: Seit dem Hörkompass-Lock öffnet ein Klick
auf einen sichtbaren Befund Ursache und Hörbeweis. Produktpositionierung:
kleines, ruhiges Objekt für
den DAUERGEBRAUCH („nie überladen") — trifft den Namen Nakama/Gefährte
wörtlich.

**VISUELLER NORDSTERN (User-Freigabe 19.08.): DER HÖRKOMPASS.** Der Mix
bleibt unsichtbar; nur dort, wo er sich selbst im Weg steht, bricht das
Prisma das Licht. Gesund = leeres Glas. Nur ein belastbarer Befund
aktiviert lokal das Schlieren-Prüffeld; das Prisma wendet die betroffene
Fläche langsam zum User. Klick = Ursache/Evidenz/Hörbeweis/ein nächster
Versuch; nach erfolgreicher Prüfung klärt sich das Glas wieder. Keine neue
Grundmetapher und kein Dauer-Visualizer. Verbindlicher Volltext samt
Fünf-Keyframe-Storyboard:
`docs/visuelles-zielbild-hoerkompass.md`.

**Darstellungs-Entscheidung (User-geprüft, Gimmick-Test):**
Film-Compositing statt Spiel-Engine. Prisma offline pfadverfolgt
(Blender 5.1 Cycles, **CPU** — Arc-Landmine!) mit echter Dispersion/
Kaustik. Ziel-Sandwich nach NAK-16: lebender Unicorn-Hintergrund (Ebene 0)
→ durch die ST-Map live gebrochenes Glasinnere → lokale Schlieren-
Befundebene → Oberflächenglanz/Kanten. Die frühere rohe
P01-Punktdatenebene ist als Dauerinhalt verworfen; P01 bleibt nur ein
Ästhetik-Datum. Drei Ruhelagen + kurze 120°-Übergänge bleiben
speicherleicht und JUCE-freundlich; Fokus- und Hörbeweiszustände werden
erst nach dem verbindlichen Storyboard konkret animiert.

**Material-Probestills GERENDERT (17.08. spät, „ausgeführt und gesehen"):**
`eq-copilot/design/prisma/prisma-material-still.py` (Blender 5.1 headless,
Cycles CPU) + `renders/prisma-{klar,rauchig,frostig}.png` (1600×1000,
1536 Samples). Aufbau: Dark-Field-Glasfotografie (schmale vertikale
Streifen-Softboxen zeichnen Kanten, Raum bleibt schwarz), Winter-Nexus-
Screenshot als Rückwand-Ebene 0, Drei-IOR-Dispersion (R/G/B-Glass-BSDFs
additiv — Blender 5.1 hat KEINE native Dispersion). **User-Urteil (18.08.):
„Klar ist zumindest auf dein bildern mein favorit, dann rauchig"** —
Arbeitsrichtung: KLAR, rauchig als Reserve, Frost raus. Vorbehalt
„zumindest auf den Bildern": endgültige Anmutungs-Abnahme am lebenden
Objekt (drehbar, mit Daten), nicht am Standbild.

**Effektdarstellungs-Festlegungen (User, 18.08. — bindend):**
1. Reine Lichtbrechung allein trägt NICHT („zu dünn") — sie ist Material,
   nicht Inhalt.
2. **Der Raum wird genutzt:** die Bühne um das Prisma (Boden/Wand/Licht)
   ist Teil der Darstellung, nicht nur Kulisse.
3. **Der Effekt pro Seite lebt IM Prisma** (im Glasvolumen des
   3D-Objekts), nicht auf ihm als Overlay.
4. Qualitätsmaß: **definiert** statt Shaderzirkus — keine generischen
   Blitzgewitter, keine chaotischen Wallpaper-Visualizer. Technikspektrum
   bewusst diskutiert (Unreal bis einfachste JUCE-Animation).

**Cycles-Grenzen, teuer erkundet (17.08., für alle Folge-Renders):**
Spot-Watt sind KUGELnormiert — enger Kegel maskiert statt bündelt
(3°-„Laser" braucht ~300 kW). Der Newton-Spektralfächer durch ZWEI
Glasflächen ist unidirektional praktisch unsampelbar (NEE geht nicht
durch Glas; MNEE/`is_caustics_*` löst nur EINE Fläche); tragfähig ist
ein kleines Flächenlicht NAH hinter dem Glas (großer Raumwinkel →
sampelbar; Quellwinkel < 4.75° Dispersionsspreizung, sonst Weiß).
Für echte Bündel-/Fächer-Sequenzen später: bidirektionaler Renderer
(LuxCore) oder Compositing-Pass.

**Unicorn-Workflow (steht, verifiziert):** User gestaltet in
unicorn.studio; `design/unicorn-bruecke.html` lädt die Szene
(`?projekt=<ID>`) und füttert pro Frame die Standard-Variablen
(bass/mitten/hoehen/kick/…, Tafel zeigt Verdrahtung, Taste T).
User-Hintergrund „Winter Nexus": ID `6YU0kFPu0uCBf0ZOTXOC`, JSON-Backup
`design/unicorn/hintergrund-winter-nexus.json`. Lizenz/Legend-Frage für
Auslieferung: NAK-11.

**EBENE 0 LIVE (19.08., User: „mach das als hintergrund"):** Aktueller
Hintergrund der Aufzeichnung ist **„Winter Nexus (Remix) 3"**, ID
`DAGhm8NIQHdX9GhNYmP9` (Backup
`design/unicorn/hintergrund-winter-nexus-remix3.json`; reine Kunst-Szene,
keine Variablen). Er liegt live und vollflächig als Ebene 0 HINTER
`prisma/prisma-schlieren.html` (Bühne) und hinter der Brücke — per
`UnicornStudio.addScene({elementId, projectId})`, bewusst NACH `init()`,
damit `szenen[0]` die Hauptszene bleibt (API im UMD v2.2.10 verifiziert;
addScene läuft auch standalone, am Blatt gesehen). Offline-sicher: CDN-
oder Szenen-Fehler werden verschluckt, der dunkle Grund bleibt.
`?hintergrund=0` schaltet ab — **für deterministische Messläufe Pflicht**
(der Hintergrund animiert frei; Element-Screenshots von `#bild` bleiben
davon unberührt). Hinter der Drei-String-Szene wird Ebene 0 erst sichtbar,
wenn deren schwarzer Gradient-Layer in Unicorn unsichtbar wird.
**FREIGESTELLT (Nacht 19.08., NAK-15 geschlossen, Commit 7de964d):**
Vorgabe-Frames sind jetzt `renders/dreh-frei/` (72× RGBA-WebP, VP8X+ALPH,
~1,4 MB): film_transparent, Plate+Boden kameraunsichtbar (bleiben Licht-
und Spiegelquellen — die Plate ist durchs Glas weiter sichtbar,
dokumentierter Ebene-0-Kompromiss), Kamera IDENTISCH zur bewiesenen
JS-Projektion (lens 65, POS −1,65/−3,53/0,95 — die Skript-Defaults sind
die STILL-Kamera, nicht die Sequenz!). Dazu `renders/dreh-boden/` (72×):
NUR der Boden mit Pfützen/Glanz, Prisma lichtwirksam aber unsichtbar,
offline in Alpha-aus-Helligkeit gewandelt — Kontakt-Ebene UNTER dem
Prisma (Taste B, `?boden=0`). **Shadow Catcher war gemessen der falsche
Weg** (0 Pfützen-Pixel — die Pfützen sind GLANZreflexe; der Catcher fängt
Schatten/Diffus). Beweise: RGB-Treue in der Silhouette Mittel-Delta 1,73
gegen f035 alt; Pixel-Probe neben der Silhouette exakt Body-Farbe (Canvas
wirklich transparent, Ebene 0 scheint DURCH das Bild); alle
Funktions-Checks grün, Blatt↔Brücke weiter bitgleich. `?frames=dreh`
lädt den alten eingebackenen Satz zum Vergleich (bleibt Archiv +
Prototyp-Quelle). Messläufe: `&hintergrund=0` bleibt Pflicht.
**NACHTRAG 19.08. — vom User VERWORFEN:** „spiegelung des alten
hintergrundes eingebacken … beleuchtung komplett falsch … billig
zusammengeklebt." Die Silhouetten-Freistellung trägt, aber das
GLASINNERE zeigt weiter die alte Plate (Transmission/Reflexion), und die
Dark-Field-Lichtwelt passt nicht zur lebenden Szene. Die „RGB-Treue
Delta 1,73" war die falsche Metrik — sie bewies das Eingebackene.
Freigegebener Nachfolger: **NAK-16 ST-Map-Live-Refraktion**
(`docs/NEXT-SESSION.md` + Geschmacksprofil „Freistellungs-Probe").

**NAK-16 PROBE-STILL GEBAUT UND BESTANDEN (19.08., cba6bd6 + 602b20a —
„ausgeführt und gesehen"):** Das Glas bricht den LEBENDEN Hintergrund.
Kette: `prisma-material-still.py --nur-stmap` (Refraction BSDF, Plate
emittiert u/v/1, filterlos Box 0,01, Denoise/Adaptive aus, 32f-EXR —
4 s/Frame, Delta-Pfad) + `--nur-glanz` (Kanten gegen Schwarz, 11 s)
→ `konvertiere-stmap.py` (Blender-Python: u=R/B v=G/B, Plate-UV → Welt
→ bewiesene Blatt-Kamera → SCREEN-Offsets float16 + Gewicht; Beweis:
Direktsicht-Offsets neben dem Prisma p99 0,019 px) → `stmap-probe.html`
(WebGL2: Karte RGBA16F/LINEAR, Composite
`mix(direkt, bg(uv+off)·B + glanz·a, glanzA)`, 3-Tap-Dispersion).
Erfolgsmaßstab der Freigabe erfüllt: 74,2 % der Glaspixel ändern sich
mit dem Hintergrund-Stand (`renders/stmap/messung.json`,
Screenshot-Paar probe-live-a/b.png, lebender Nexus
probe-live-unicorn.png). Checkliste beantwortet: Refraction BSDF ist
NICHT Fresnel-gewichtet (B≈1; Fresnel liegt komplett im Glanz-Pass) ·
TIR 12 % der Glasfläche (zeigt ehrlich nur Glanz) · 64 Samples genügen
(Seed-Rauschen p99 0,037 px) · Half-Float trägt (max 0,19 px) ·
SwiftShader-Boden 53 ms/Frame (echte GPU deutlich schneller).
**Wichtig für die Blatt-Integration:** Ziel-UVs reichen bis u=1,5 —
Ebene 0 braucht ÜBERSTAND übers Glasfenster (Probe: Hintergrund
1920×1280, Glas 768×1024 zentriert); die Offsets sind in
Glas-Canvas-UV und werden affin in Hintergrund-UV umgerechnet.
Offen: User-Sichtung am lebenden Blatt (Dispersion `?dispersion=`,
Glanz-Rig), dann 72er-Sequenz + Integration.

**FALLEN aus dem NAK-16-Bau (jede gemessen, nicht vermutet):**
1. **Fremde WebGL-Canvases lesen sich SCHWARZ** (texImage2D), wenn sie
   ohne `preserveDrawingBuffer` erstellt wurden — der Unicorn-Canvas
   braucht den getContext-Patch VOR dem SDK-Load (steht in
   `stmap-probe.html`; bei Integration mitnehmen).
2. Ein per JS erzeugtes `<script>` lädt erst mit `appendChild` — der
   stille Unicorn-Ausfall war GENAU das (kein Request, kein Fehler).
3. Unicorn rendert nicht in off-screen positionierte Elemente
   (left:-9999px) — Ebene 0 sichtbar HINTER dem deckenden Composite
   platzieren.
4. **Spiegelwinkel nie aus der Eckenliste herleiten:** die
   Seiten-Normalen des Prisma-Meshes zeigen nach INNEN (NAK-17) und
   die Fase ist flat-shaded (diskrete Sub-Normalen) — Licht-Rigs am
   evaluierten Depsgraph-Mesh messen (`--nur-glanz`-Kommentar).
5. `gl.finish()` blockt unter ANGLE/SwiftShader nicht ehrlich —
   Benchmarks brauchen readPixels als Pipeline-Zwang (0,06 ms
   „gemessen" vs 53 ms echt).

**FALLE (18.08., teuer bezahlt): Wer in Unicorn Studio einen REGLER von Hand
verschiebt, LÖST damit seine Variablen-Bindung.** Danach zeigt das
Variablen-Panel weiter Werte an, die den Shader nie erreichen — der Effekt
sieht schlicht kaputt aus. Deshalb strikt trennen: datengetriebene Regler
(`befundOrt`/`befundStaerke`/`befundBreite`/`befundTiefe`/`befundVorzeichen`/
`bandFarbe`) werden NUR über die Variable verändert, nie am Regler;
Design-Konstanten (Line Count, Grain, Speed, Adjust-Ebene) nur am Regler,
ohne Bindung. Fehlerbilder unterscheiden sich (SDK v2.2.10 gelesen):
ein unbekannter Variablen-NAME ist still (`setVariable()` wirft nicht, es
liefert nur eine „Variable anlegen"-URL zurueck), eine kaputte BINDUNG dagegen
warnt in der Konsole („Unable to apply Unicorn Studio variable binding …").
Eine an den FALSCHEN Regler gebundene Variable ist wieder still — genau der
Fall, der 18.08. Zeit gekostet hat (`befundStaerke` hing an *Probe Center*
statt an *Deflection*).

**FALLE, zweite Form (19.08., an `aaCqqL8FX4EQcCNad9Jq` gemessen): NEUE
Effekt-Layer erben KEINE Bindungen.** Die Drei-String-Layer-Szene hatte
alle 7 Variablen-DEFINITIONEN (Manifest meldet sie, IDs identisch zur
Referenz — Szenen-Kopie kopiert Definitionen), aber `bindings: []` leer,
`data.uniforms: {}` leer, und im kompilierten GLSL standen alle
Befund-Werte als Literale (`befundStaerke` wörtlich `0.0000`).
Live-Beweis: `setVariable(befundStaerke 0→1, ort 0.5→0.72)` änderte 0 von
614 400 Pixeln. **Diagnose am Export:** `variables.bindings` muss je
gebundener Property einen Eintrag haben (Referenz: 7 Stück, Ziel-Targets
`ort/staerke/breite/tiefe/vorzeichen/linien/lineColor`), und das Manifest
liefert `bindingCount` pro Variable. Die Brücken-Tafel prüft
`bindingCount` seither selbst und zeigt „⚠ definiert, aber UNGEBUNDEN"
statt eines falschen „✓ verdrahtet". Bei MEHREREN Layern muss JEDER Layer
seine eigenen Bindungen bekommen. Sicherung des ungebundenen Stands:
`design/unicorn/drei-string-layer-szene-2026-08-19-ungebunden.json`.
Achtung zusätzlich: der KI-generierte Drei-Layer-Effekt rechnet
`disp = -staerke·d·bump·0.08` (fester Gain statt kalibriertem `w·0.76`)
— die Ablesungs-Kalibrierung der Referenz-Szene (Deflection 100 → Lücke
195 %) gilt für diese Layer NICHT, selbst nach dem Binden.

**Wertsemantik geklaert (SDK v2.2.10):** `applyBinding()` schreibt per
`setRuntimeOverride`/`setPropertySource` — die Variable ERSETZT den
Reglerwert, sie skaliert ihn nicht; der Ausgangswert des Reglers ist nach
dem Binden bedeutungslos. Die Umrechnung passiert nur fuer `color` (Hex mit
oder ohne `#`, 3/6/8-stellig, → Vec3 0–1); jeder andere Typ wird UNVERAENDERT
durchgereicht — es gibt KEINE Prozent-Umrechnung zur Laufzeit. Der
Variablentyp „Percent" ist reine Editor-Kosmetik. Gilt fuer die Laufzeit
(Embed); die Editor-Vorschau ist anderer Code und ungeprueft.

**SICHT-PROBE 18.08.: DIE SCHLIEREN-ABLESUNG** (`eq-copilot/design/prisma/
sicht-probe-schlieren.html`, Renders `renders/schlieren/`) — Ergebnis eines
/diverge-duo-Laufs (Claude + Codex blind aufgefächert, gegenseitig
angegriffen). **Der Befund wird NIE gezeichnet.** Sichtbar ist nur, wie er
ein Prüffeld gerader Lichtlinien IM Glasvolumen verbiegt (Schlieren-
Verfahren). Begründung: eine Resonanz ist kein Stoff, sondern eine lokale
Änderung des Antwortverhaltens — sichtbar nur an ihrer Wirkung auf eine
Sonde. Ablesegrammatik: betroffene Linien = fVon..fBis · Versatz in
KAMM-EINHEITEN = Stärke (der Kamm ist zugleich das Lineal) · senkrechte
Ausdehnung = Persistenz-Anteil · Überschuss bündelt, Fehlbetrag spreizt.
Ablenkung = GRADIENT der Abweichung gegen die eigene Schulterlinie, deshalb
ist die Silhouette bewusst KEINE EQ-Kurve.
Gemessen und gesehen: Schulterlinie muss über eine OKTAVE mitteln (bei
halber Oktave frisst die Schulter den Buckel, max. Abweichung 1,45 dB);
Prüfton +6 dB @ 116 Hz ergibt Befund 115 Hz / 104–126 Hz / +4,6 dB /
Persistenz 97 % und kneift den Kamm sichtbar.
**Entschieden im Hörkompass-Lock 19.08.:** Im gesunden Zustand steht das
Prüffeld NICHT dauerhaft; gesund = leeres Glas. Die Produktansicht zeigt
evidenzbasierte Befunde, nicht jeden kleinen Rohgradienten der Musik. Der
alte Modus „Feld nur bei Befund" erfüllt das technisch noch nicht — er
räumt das Glas wegen der Rohgradienten nicht frei und muss später gegen
die Befundschwelle statt gegen bloße Feldaktivität verriegelt werden.
**Nicht dargestellt:** Konfidenz (der Render hat ZWEI Streifen-Softboxen,
nicht vier — die Vierer-Ablesung wäre eine Behauptung über ein Bild, das es
nicht gibt), Bündel-Modus, Mehrfachbefunde (NAK-14).

**SCHLIEREN-EFFEKT IN UNICORN — INSTRUMENT KALIBRIERT (18.08., gemessen):**
Projekt `ilYX64xlvU811imBKfxV` („Copy of Untitled project"), Effekt „Schlieren
Probe Field" (KI-Effekt-Editor, iterativ nach Mess-Prompts korrigiert).
Regler: Probe Center · Deflection · Zone Width · Vertical Extent · Convergence ·
Line Count · Grain Intensity · Line Color · Speed. Gebunden an die Variablen
`befundOrt`/`befundStaerke`/`befundBreite`/`befundTiefe`/`befundVorzeichen`/
`bandFarbe` (Typ Percent bzw. Color); Line Count/Grain/Speed bleiben ungebunden
= Design-Konstanten.

Abnahme am Export (1440x900, Line Count 24, Zone Width 30, Nennabstand ~79 px):
- Deflection 100: groesste Luecke 153,5 px = **195 %**, engster Abstand
  60,5 px = **77 %** — keine Ueberlappung, zaehlbar.
- Deflection 40 (Normalfall, ~2,4 dB): groesste Luecke 106 px = **134 %**.
- **Antwort strikt linear** (Ueberschuss 26,7 px bei 40 vs 74,9 px bei 100).
- **Spiegelsymmetrie exakt 0,0 px** ueber fuenf Linienpaare, bei 40 wie bei 100.
- **Mittellinie steht** (719,5 bei Bildmitte 720,0) — der Beweis, dass der Shader
  die ABLEITUNG der Abweichung rechnet und nicht die Glocke selbst.

STAND 18.08. abends: Das Feld liegt IM GLAS
(`prisma/prisma-schlieren.html`, WebGL auf einer 3D-projizierten Ebene,
Beschnitt durch den Glaspass). Farbe abgenommen (uLineColor = E8A34C, am
Szenen-Export belegt). USER-URTEIL: „das einzige was an Animation oder
Bewegung sichtbar ist, ist ein Pixelzucken" — Bewegung war der offene Punkt.

**BEWEGUNGS-DIAGNOSE (18.08. spät, gemessen — Commit 7148248):**
- **lineIdx-Verdacht WIDERLEGT** (Flimmern durch Linien-Identität aus der
  verschobenen Koordinate): Frame-Paar 1/60 s bei Δstaerke 0,032 → 5330
  Pixel Subpixel-Schimmer, exakt 1 Pixel Vollausschlag; Noise-Floor
  (identische Zeit zweimal) exakt 0. Aus dem Code zusätzlich hart bewiesen:
  ein Punkt leuchtet nur bei distX < 0,30·spacing (Deckel in halbBreite),
  lineIdx kippt erst bei 0,5·spacing — ein SICHTBARER Punkt kann seine
  Identität nicht wechseln. Der notierte Fix (Kamm aus verschobenen
  Linienmitten) ist unnötig; NICHT umbauen.
- **Wahre Ursachen:** (a) Der Prüfton STAND bei 116 Hz — Stillstand war die
  korrekte Anzeige; (b) **Bin-Treppe:** ort/breite/fVon/fBis waren auf
  1/96-Bins gerastert (ort in Folgeframes identisch trotz wachsender
  Stärke) — echte Wanderung wäre in ~2-px-Stufen gesprungen.
- **Eingebaut (Blatt + Brücke, bitgleich — befundOrt beider Seiten
  identisch verifiziert):** Sub-Bin-Interpolation (Parabel-Spitze, linear
  geschnittene Halbwertskanten, Persistenz am interpolierten Ort) und die
  **Prüfton-Fahrt**: der deklarierte Ton entsteht bei 6 s, wandert (log,
  smoothstep 9–23 s), verschwindet ab 23 s. Statuszeile nennt live Hz und
  EFFEKTIVE dB (db·env); `?fahrt=0` oder explizites `?ptonHz=` stellt ihn
  still; Taste F schaltet die Fahrt. **19.08., User: „bass sieht okay aus,
  höhen bewegt sich nicht" → Fahrt bandrelativ (Commit 168feca):** der
  Fahrweg ist in Feld-Koordinaten definiert und gilt im aktiven Band —
  Band 0 55→150 Hz, Band 1 ~530→1450 Hz, Band 2 ~5,2→14 kHz (vorher lag
  der feste Hub nur in Band 0; Mitten/Höhen standen ehrlich still, weil
  die Schleife dort nur schulterbreite Anteile hat). Bandwechsel baut die
  Persistenz jetzt neu auf (15 s Vergangenheit deterministisch nachgerechnet
  — vorher leuchtete der Verlauf des alten Bandes ~5 s als falsche Tiefe
  nach). Blatt↔Brücke weiter bitgleich verifiziert. Gemessen danach:
  ort gleitet jeden Frame (~19 % eines Bins), fMax zählt kontinuierlich
  (80,5→80,8→82,7 Hz über 0,15 s); die Störung ist ein kompakter Wirbel
  (Persistenz eines Wanderers ehrlich ~23 % statt 95 %), der das Feld in
  ~14 s quert. **Optik-Abnahme am lebenden Blatt: offen (User).**

FRUEHER OFFEN: (a) Farbe — gemessen rgb(189,169,121) statt E8A34C, und der Farbton
wandert mit der Helligkeit (bei Alpha 128 → rgb(255,255,241)); Ursache noch
nicht getrennt (Adjust-Ebene liegt drueber). (b) SUBSTANZ — es sind weiterhin
duenne Striche; das User-Urteil „trostlos" ist damit noch nicht beantwortet.
(c) Der Effekt lebt noch nicht IM Prisma. (d) Bruecke zeigt noch auf die alte
Projekt-ID `tRg2ubAR1fLKQqov1YLd`.

**Geschmacks-Kalibrierung (abgeschlossen, 4 Proben unter
`design/proben/`):** P01 Perkolation ✓ abgenommen · P02 Lichtschleier ✗ ·
P03 Tropfenklingen ✗ · P04 Interferenz ✗. Die drei Gesetze + Verschärfung
3a–3c + **Meta-Erkenntnis** (Vision kommt vom User; Claudes freie
Bilderfindung = Mode-Collapse) stehen wörtlich in
`docs/geschmacksprofil.md` — **vor jeder Design-Entscheidung lesen.**

## Vorheriger Richtungswechsel 17.08.: Kunstwerk statt Graph

User-Entscheid: Das Tiefenfeld (2D-Graph, unten dokumentiert) ist
**GEPARKT — nicht verworfen**; Dateien und Bauplan 2.0 bleiben unverändert
liegen. Neues Ziel: Der Nutzer sieht seine Musik als **Kunstwerk, mit dem
er interagiert** — kein wabernder Analysegraph. Entschieden (AskUser):
Bauplan-2.0-Grammatik ist mitgeparkt (nur das Grundgesetz bleibt bindend) ·
Hineinhören (bestehende Hör-Markierung) gehört ins Interaktionskonzept ·
das Kunstwerk IST die Hauptansicht, kein Zweitmodus.

**Neues Blatt:** `eq-copilot/design/nakama-kunstwerk-claude.html` — drei
umschaltbare Bildwelten (Tasten 1–3, `?welt=`) auf derselben 30-s-Songschleife,
jede mit strikter Eins-zu-eins-Messgrammatik (Legende im Blatt):

1. **GARTEN** — Musik wächst als Beet über der Log-f-Achse. Halmhöhe =
   Energie-Gedächtnis · Wiegen = Jetzt · Blattdichte = P50 · Flackern =
   P95−P50 · dunkle Blüte = Resonanz (Puls = Persistenz) · kahle Stelle =
   Loch · Moos = Mulm · Bodennebel = fehlende Abdeckung · Fähnchen = Befund.
2. **GEWEBE** — die Session webt das Lied: 1 Reihe = 1 s Aktivzeit,
   Tintendichte = Energie, Knoten = Resonanz-Ereignis, Kettfaden =
   Persistenz, fadenscheinig = Loch, verfilzt = Mulm, Webkante =
   Stereobreite. Das Arrangement wird als Stofftextur sichtbar.
3. **WESEN** — der Mix ist EIN Lebewesen: Winkel = Frequenz, Kontur =
   Gedächtnis, Atem = Lautheit, Herzschlag = Kick, Stacheln =
   Resonanz/Härte, Delle = Loch, Kernfarbe = Klangschwerpunkt.

**Hineinhören als Interaktion:** Befund-Element anklicken → Karte
(Messdaten + Empfehlung) mit SOLO/PULS — Zwei-Pass-Render: Szene hinter
dunklem Schleier, nur das gewählte Element leuchtet (Puls im
250-ms-Raster). `?hoeren=reso-low:solo` als Deep-Link. Loch kann nicht
pulsen (wie im Plugin). Kein Maskierungs-Befund im Blatt (Paar-Phänomen
des Brokers, in Solo-Ansicht nicht ehrlich zeigbar).

**Beweise:** `node eq-copilot/design/render-kunstwerk.mjs` → `k1-*`-Satz
(Seitenfehler = Abbruch); Welten-/Solo-/Leerzustand-Renders gesehen.
**Nächster Schritt (liegt beim User):** Blatt im Browser öffnen, Richtung
wählen — Claudes Empfehlung: GARTEN vorn, GEWEBE als zweite Kraft.

Nicht gegangener Weg (notiert, nicht verworfen): „Schriftrolle" — die
Session schreibt asemische Schriftzeichen je 10-s-Fenster; als Hauptansicht
zu informationsarm, als Beiblatt/Export-Idee weiter denkbar.

## Die zwei Ebenen

1. **Zielvertrag (verbindlich):** `eq-copilot/docs/NAKAMA-SPECTRAL-FIELD-BAUPLAN.md`
   — „Bauplan 2.0" für die neue VST3-Hauptansicht. Nakama = bildschirmfüllendes
   Spectral Field: Membran-Kurve, textfreie Symbolkreise mit Dringlichkeitsring,
   überlagerbare Befunde (kein Top-Drei-Limit), manuelles Befundarchiv,
   umschaltbare Farbpakete, 仲間-Bildmarke als Pfadgeometrie, Schriften
   Nakama Phase + Manrope (eingebettet). Kern-Grammatik §6.6: **jede sichtbare
   Schicht hat genau eine Messbedeutung**; Nebel und Licht sind zwei Enden
   EINER Evidenzachse (`nebel_anteil + licht_anteil = 1`). Es gibt keinen
   globalen Zielkorridor — `ltasReferenzDb` ist Messachse, keine Sollkurve.
   Der Umbau der produktiven JUCE-Ansicht ist **noch nicht erfolgt**.

2. **Live-Design-Fläche (hier passiert die Arbeit):**
   `eq-copilot/design/nakama-spectral-field-claude.html` — das
   **Claude-Musterblatt** im Browser. Design-Schleife = Sekunden (Regler
   ziehen), Plugin-Schleife = Minuten (Kompilieren + UAC + FL-Neustart);
   deshalb bleibt die Design-Phase im Musterblatt, bis der Design-Freeze
   fällt (Transfer-Vertrag im 3D-Effekte-Plan §5.1).
   `nakama-spectral-field-vorentwurf.html` ist **Codex-Besitz** — nie
   editieren (PreToolUse-Guard blockt das).

## GEPARKT seit 17.08.: das TIEFENFELD (Stand beim Parken)

Nach zwei Verwürfen gilt: **EIN Mechanismus** — die Abweichung vom
Bleistift-Plan wird Relief; der Graph selbst erzählt die Befunde.

- **Nur noch dunkle Welten** (User-Entscheid 17.08.: „Weiß fliegt raus"):
  `tusche-dunkel` = Standard, `nebel-nacht` als zweite; Hell-Paletten gelöscht.
- Auf dunklem Blatt erzählt **Licht statt Schatten**: `tiefenLicht`
  (Lichtsäule unter dem Plan, so tief wie der Befund, 1:1 px) · `talLicht`
  (Lichtkante unter der Plan-Lippe) · `strichSchein` (Lichthof statt
  Schlagschatten).
- Vokabular: **Strich** = Ist (hungert im Tal) · **Bleistift** = Soll ·
  **Fahne** = Abweichung · **Befund-Nebel = WO** (Fog über dem Band,
  Dicke = Schwere, wogt nur live) · **Pool** = Zeitgedächtnis.
- **Chips = Befund-DATEN** (Resonanz · Mulm · Loch · Härte; Tasten 1–4,
  `?aus=resonanz,…`) — Kurve, Fahne, Nebel und Licht antworten gemeinsam.
- **30-s-Songschleife** (17.08., User-Wunsch „bewegt sich zu wenig"):
  deterministische Musiksimulation — Arrangement (Intro · Strophe · Chorus ·
  Ausklang), Kick/Snare/Hats bei 96 BPM, achtstufiger E-Moll-Basslauf.
  Wandert in Plan UND Strich gleichermaßen, damit die Fahne exakt die
  Befund-Abweichung bleibt; `?zeit=` friert jeden Moment reproduzierbar ein,
  `prefers-reduced-motion` legt den Beat-Puls still.
- Kalibrier-Panel: 10 Regler (u. a. `nebel`, `feld`, `schatten`) +
  **Farbwahl je Graph-Element** (10 Picker, pro Welt gespeichert; das
  Kopier-JSON trägt `farben` mit). `?kali=auf` · `?zeit=` friert ein.
- Renders: `node eq-copilot/design/render-blatt.mjs` → deterministischer
  `vorentwurf-renders/n4-*.png`-Satz (semantische Zustände + die drei
  vertraglichen Referenzgrößen); `… pruefe` fährt sechs Semantik-/
  Gegenpfadchecks (Nebel↔Licht, Leerzustand, Resize-Klassen, Stop↔Start).
  Ältere `t3-*`-Serien bleiben als Verlaufsbelege liegen.

### Falls das Tiefenfeld je entparkt wird

Musterblatt im Browser öffnen; „Werte kopieren"-JSON = Design-Freeze →
1:1 JUCE-Voreinstellung → Umbau nach Bauplan §13. Bauplan 2.0 ist mit dem
Tiefenfeld geparkt; für die Kunstwerk-Richtung gilt er NICHT.

## Verworfene Richtungen — nie neu vorschlagen (teuer bezahlt)

1. **3D-Papier-Shader-Modell (E0–E5, 16.08.):** three.js-Bühne, Brand/Riss/
   Feuchte-Felder, selektives Bloom. War vollständig gebaut und formal
   bewiesen (Null-Beweis 0,16 %, Regler-Beweis 18/18) — User: „sieht sehr
   schlecht aus". Lehre: **Pixel-Beweise messen Änderung, nie Schönheit.**
   Prozedurale Material-Simulation ist an dieser Aufgabe **zweimal**
   gescheitert. Verlaufsbeleg: `eq-copilot/docs/NAKAMA-PAPIER-3D-EFFEKTE-PLAN.md`
   (Status ⛔ VERWORFEN).
2. **Tusche-Einzelmarken (17.08.):** Bruch-Borsten („zerissenes Stroh"),
   Siegel („Kochhut") — Einzelmarken-Piktogramme lesen sich als Deko.
   Tragfähig ist, was der Graph selbst mit Licht/Tiefe erzählt und dabei
   Position/Breite/Stärke abbildet (FabFilter-Referenz: Tiefe als Mittel).

## Produktions-UI heute (zum Vergleich)

Das installierte Plugin (0.3.0, M3a) trägt noch die **Material-Kit-Front**:
warme Metallfläche + schwarzes Analyseglas + Ember-Akzent, eigenständige
Vektorfront aus `plugin/src/EqCopilotAssetKit.h` (Vertrag:
`design/ASSET-KIT.md`), Fensterformat 750:520. Diese Front wird durch das
Spectral Field **ersetzt**, sobald der Design-Freeze steht.
