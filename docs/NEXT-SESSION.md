# NEXT-SESSION — Einstieg für die nächste Runde

> **WORKSPACE-UMZUG 18.08.2026:** Alles lebt jetzt HIER
> (`Projekte\Nakama`) — eq-copilot/, broker/ (eigenständig,
> eqcop-broker.exe), tools/. FL-Repo-SHAs in älteren Einträgen unten
> gehören zur FL-Studio-Historie (bis `7964777`).

> Stand: 2026-08-17, Session-Ende. Diese Datei ist der Schnellstart;
> Tiefe in `docs/design-stand.md` und `docs/geschmacksprofil.md`.

## Der eine nächste Schritt

**DAS FELD BEWEGT SICH NICHT — nur Pixelzucken (User-Urteil 18.08., offen).**
`eq-copilot/design/prisma/prisma-schlieren.html` steht und zeigt die
vollständige Idee: abgenommenes Cycles-Prisma, Schlieren-Prüffeld im
Glasvolumen, Befund nie gezeichnet, Ablesung aus echten Zahlen. Was fehlt,
ist sichtbare BEWEGUNG. Zwei getrennte Ursachen, beide unbestätigt:

1. **Flimmern statt Struktur (Verdacht, am Code hergeleitet, NICHT gemessen):**
   `lineIdx` wird aus der VERSCHOBENEN Koordinate berechnet
   (`floor(x / spacing + 0.5)`). Ändert sich die Ablenkung, kippt ein Fragment
   auf die Nachbarlinie, damit springen `phase` und `rnd` — die Punkte poppen.
   Fix-Richtung: Linien-IDENTITÄT aus der UNVERSCHOBENEN Koordinate nehmen und
   den Kamm aus den verschobenen Linienmitten aufbauen. Dann wandert die
   Körnung MIT der Linie, wie eine Gravur im Glas es tun müsste.
2. **Es passiert schlicht zu wenig:** Der Prüfton steht konstant bei 116 Hz,
   der Befund ändert sich über die 30 s kaum. Ohne Prüfton liefert die
   Songschleife < 2 dB und das Feld steht völlig still. Für eine ehrliche
   Bewegungsprobe braucht es einen Befund, der über die Zeit WANDERT und
   verschwindet — sonst ist Stillstand die korrekte Anzeige.

Erst 1 messen (Einzelbilder zweier aufeinanderfolgender Frames vergleichen),
dann 2 entscheiden. Nicht beides gleichzeitig ändern.

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
