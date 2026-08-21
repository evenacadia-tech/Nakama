# Prisma-Studie — Statusblatt

**Status: geparkt (User, 21.08.2026).** Kein Produktteil, kein Bauplan, kein
Ticket. Nichts hieraus ist Vorgabe für Gen, Probeeq oder Suna; ihr Vokabular
(Glas, Licht, Schlieren, Hörkompass) gehört nicht in die Plugin-UI.

## Die User-Worte, an denen dieser Status hängt

| Datum | Wort |
|---|---|
| 17.08. | Prisma-Richtung begonnen (Herkunft lt. User 21.08.: **„Meine Idee"**) |
| 18.08. | „das prisma sieht top aus … wasserfall aus farbigen pixeln" → Objekt ✓, Punktwolke als Dauerinhalt ✗ · Material: „Klar ist zumindest auf dein bildern mein favorit" |
| 19.08. | Nacht-Freistellung: „tut mir leid aber das war keine gute arbeit … eingebacken" ✗ · ST-Map-Refraktion zum Bau freigegeben („ja aber das rendern machen wir nächste session") |
| 19.08. | Hörkompass-Text (aus einem Idea-Generator): „Das klingt sehr durchdacht, gefällt mir" · „Ja genau" |
| 20.08. | Prisma = eigenständige Begleit-App neben Master-Plugin + Sonden (User 21.08.: „Meine Entscheidung, so gesagt") |
| **21.08.** | **„Familie; Prisma nur Studie"** · Hörkompass: **„Alles nur Studie"** · Glas/Licht: **„Glas/Licht raus; Profil nur Studie"** · Inspirationen: „Weiß nicht mehr – raus" |

## Was hier liegt (Stand beim Parken)

| Ordner / Datei | Was es ist | Stand |
|---|---|---|
| `prisma/prisma-prototyp.html` | drehbares Prisma aus vorgerenderten Cycles-Frames; UI-Zeile „SICHT-PROBE: P01-PERKOLATION IM GLASVOLUMEN" (die Punktwolke ist als Dauerinhalt verworfen — das Blatt zeigt sie trotzdem standardmäßig; Kopf behauptet „ohne Datenebene") | Denk-Leinwand 18.08.; Widerspruch dokumentiert, nicht behoben |
| `prisma/stmap-probe.html` + `prisma/renders/stmap/` | ST-Map-Live-Refraktion (NAK-16): Probe bestanden (74,2 % der Glaspixel folgen dem Hintergrund, `messung.json`); 72er-Drehsequenz | gebaut 19.08.; **Optik-Abnahme nie erfolgt** |
| `prisma/prisma-schlieren.html`, `sicht-probe-schlieren.html`, `schlieren-probe-field.frag` | Schlieren-Prüffeld (Befund = Verbiegung gerader Lichtlinien) | Sicht-Probe; nie abgenommen (User-Urteile nur „Pixelzucken", „trostlos", „bass sieht okay aus") |
| `prisma/vorlage-lichtwelt.html`, `vorlage-tiefenkarte.html`, `docs/spielregeln-skizze.html` | Orientierung für die Unicorn-Lichtwelt (Ebene 0) | Werkzeug |
| `prisma/prisma-material-still.py`, `konvertiere-stmap.py`, `miss-stmap.mjs` | Blender-5.1-Cycles-Renderer (CPU!), ST-Map-Konverter, Messung | Werkzeuge; Pfad zu `../../vendor-build` beim Umzug nachgezogen |
| `prisma/renders/` (`dreh`, `dreh-glanz`, `dreh-boden`, `dreh-frei` ✗ 19.08., `dreh-stmap` gitignored, `schlieren`, `stmap`, Stills) | Bildbeweise und Frames | bleiben als Beweis; **entfernt 21.08.:** `stmap/stmap-daten.js` (8,3 MB) und `stmap/dreh-karten.bin.gz` (17,4 MB) — regenerierbar |
| `unicorn/` (4 JSON), `unicorn-bruecke.html`, `vendor/unicornStudio-2.2.10.umd.js` | Unicorn-Szenen (Winter Nexus, Drei-String, Schlieren-Feld), Brücke Musikdaten → Unicorn, lokal gevendortes SDK | NAK-11 (Unicorn-Lizenz) bleibt offen, solange die Studie geparkt ist |
| `docs/design-stand.md` | Design-Gedächtnis 13.–20.08. (beide Produkte übereinander geschichtet) | Verlauf |
| `docs/geschmacksprofil.md` | vier Proben + wörtliche User-Urteile (Schärfe, Handschrift, Notwendigkeit, Vision vom User) | **gilt nur für diese Studie** (User 21.08.); die Meta-Lehre „Claudes freie Bilderfindung = Mode-Collapse" lebt als Arbeitsregel in CLAUDE.md weiter |
| `docs/visuelles-zielbild-hoerkompass.md` | Hörkompass-Zielvertrag (Idea-Generator-Text, gebilligt) | **„Alles nur Studie"** |
| `docs/2026-08-19-stmap-live-refraktion.md` | Pipeline-Doku der ST-Map-Refraktion | Verlauf |

## Wenn die Studie je wieder aufgenommen wird

1. Der User sagt es ausdrücklich; dann neuer Freigabe-Marker mit seinem Wortlaut.
2. Regenerieren: `prisma-material-still.py --nur-stmap --dreh 72` (≈ 5 min, CPU) ·
   `--nur-glanz --dreh 72` (≈ 13 min) · `konvertiere-stmap.py --dreh 72` → erzeugt
   `renders/stmap/dreh-karten.bin.gz` und `stmap-daten.js` neu.
3. Pfade in `docs/*.md` meinen den Stand vor dem Umzug (`eq-copilot/design/prisma/…`).
4. Nie GPU-Batch-Loops auf der Arc A770 (Lüfter-Failsafe); Cycles auf CPU.
