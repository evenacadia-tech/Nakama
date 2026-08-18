# Design-Stand — an welchem Prototypen wir arbeiten

> **Stand: 2026-08-17 (abends).** Lebendes Dokument — nach jedem
> Design-Commit im FL-Studio-Repo hier nachziehen. Bei Widerspruch gewinnt
> der Code (`eq-copilot/design/…`) und der Bauplan dort.

## AKTIVE RICHTUNG (Stand 17.08. abends): DAS PRISMA

Nach der Proben-Kalibrierung (unten) hat der **User selbst** das Konzept
autorisiert — und es ist das tragfähigste des Projekts:

**Ein dreiseitiges, drehbares Prisma als kompakter Dauerbegleiter.**
Drei Flächen = drei Blickrichtungen (Tiefen / Mitten / Höhen); Drehen ist
SELEKTION („was will ich gerade sehen"), kein Deko. Klick = Bündel-Modus:
die zerlegten Bänder rekombinieren zur Gesamtsicht (Newton: zweites
Prisma bündelt Spektrum zu Weiß — physikalisch echt, mathematisch die
Inverse der Analyse). Produktpositionierung: kleines, ruhiges Objekt für
den DAUERGEBRAUCH („nie überladen") — trifft den Namen Nakama/Gefährte
wörtlich. Offen (User-Vorstellung): was auf einer Fläche lebt, was das
Gebündelte formt (Kandidaten: Klangkörper-Silhouette · Balance-Ring ·
P01-Perkolation im Glas).

**Darstellungs-Entscheidung (User-geprüft, Gimmick-Test):**
Film-Compositing statt Spiel-Engine. Prisma offline pfadverfolgt
(Blender 5.1 Cycles, **CPU** — Arc-Landmine!) mit echter Dispersion/
Kaustik; zur Laufzeit Ebenen-Sandwich: Unicorn-Hintergrund (Ebene 0,
mausreaktiv) → live gerechnete Daten-Ebene (P01-Punktästhetik),
perspektivisch auf Flächen gewarpt → vorgerenderter Glas-Pass obendrauf.
Nur 3 Ruhezustände + kurze Übergangssequenzen (120°-Snap, Bündeln) —
speicherleicht, JUCE-freundlich (Bilder + kleine Live-Schicht, kein
GPU-Zwang).

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
