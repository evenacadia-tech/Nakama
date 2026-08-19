# Offene Punkte — durables Offen-Set

> Regeln: Offene Einträge werden beim Auffrischen NIE gelöscht. Schließen
> nur mit ID + Commit-SHA (oder expliziter User-Entscheidung). Neue
> Out-of-scope-Funde kommen als datierte Zeile mit ID dazu.
> Nur Nakama-/eq-copilot-Punkte — FL-Studio-Workspace-Themen (MIDI,
> Vorlagen, Plugin-Hub-GUI) bleiben dort.

## Offen (Stand 2026-08-17)

| ID | Seit | Punkt |
|---|---|---|
| NAK-01 | 08-17 | **Design-Freeze ausstehend:** User kalibriert das Tiefenfeld-Musterblatt im Browser; „Werte kopieren"-JSON in den Chat = Freeze → JUCE-Voreinstellung. |
| NAK-02 | 08-17 | **JUCE-Umbau der Hauptansicht** nach Bauplan 2.0 §13 (12 Schritte) — startet erst NACH NAK-01. |
| NAK-03 | 08-15 | Beim User: FL-TESTANLEITUNG §11–17 (M2-Roundtrip · M3-Karten · Graph/„KURVE STEHT") + §18 Hör-Markierung (Puls/Solo hören, Render-Beweis). Plugin 0.3.0 ist installiert. |
| NAK-04 | 08-15 | 16-Instanzen-/PDC-/Offline-Render-Verhalten = Alltagsbeobachtung, User meldet formlos. |
| NAK-05 | 08-14 | M3b: Korridor-Kalibration aus even-Bounces (Benchmark-Studie, nächste Stufe). |
| NAK-06 | 08-14 | M3c: PRE/POST-Karte, Hover-Inspekt, Passagen, Masking (Benchmark-Studie). |
| NAK-07 | 08-14 | Dauerbetrieb: Loudness inkrementell, Pipe-Deadlines. |
| NAK-08 | 08-14 | Claude-Anbindung: `--bare` scheitert an OAuth → Agent-SDK-/API-Adapter nötig (M4-Thema; Grundgesetz: Aufruf nur auf bewussten Klick). |
| NAK-09 | 08-15 | Kalibrier-Protokoll Runde 2 (helle Instrumente ODER EQ-Fehler-Blindtest), `eq-copilot/kalibration/`. |
| NAK-10 | 08-17 | Broker liest das `hoermarkierung`-Heartbeat-Flag noch nicht (serde verwirft still; `protokoll.rs` MessStand ohne Feld) — Aggregat soll laut `eq-ipc.schema.json` während Markierung pausieren/markieren. Bewusst nach dem Harness-Lauf, s. HOER-MARKIERUNG-KONZEPT. |
| NAK-11 | 08-17 | Unicorn-Studio-Weg: Offline-/Selbsthosting der Szenen-JSON braucht den „Legend"-Plan; SDK-Lizenz („nur für legitime Unicorn-Studio-Projekte") vor einem JUCE-WebView-Einsatz im ausgelieferten Plugin klären. Design-Phase (CDN-Embed) ist unkritisch. |
| NAK-12 | 08-18 | Sensorübersicht heimatlos: Das Svelte-Panel (Profilbindung, Sensor-Status) flog mit der Broker-Herauslösung aus der Hub-App; Referenzkopie in `broker/sensoruebersicht-referenz/`. Braucht ein neues Zuhause (eigene Mini-UI, CLI oder Plugin-Editor) — bis dahin Bindungen nur per JSON-Datei + Broker-Neustart. |
| NAK-13 | 08-18 | Broker-Autostart fehlt: Früher startete die Hub-App den Broker automatisch; jetzt muss `eqcop-broker.exe` von Hand laufen, sonst misst das Plugin nur lokal (verbindet per Backoff, sobald der Broker da ist — kein Datenverlust, aber keine Paare/Aggregate). Klären: Autostart-Eintrag, Task-Scheduler oder Start auf Zuruf. |
| NAK-14 | 08-18 | Lesbarkeits-Test der Befund-Verkörperung ist bisher nur an EINEM Befund gedacht: Kein Entwurf aus dem /diverge-duo-Lauf (12 Optionen) wurde gegen den Alltagsfall MEHRERE gleichzeitige Befunde (3–4 nebeneinander, ggf. auf verschiedenen Flächen) geprüft — genau der Fall, für den der 2-Sekunden-Massstab des Profis gilt. Ebenso ungenutzt als Träger: die drei Flächen/das Drehen und der Bündel-Modus. Vor dem Bauen einer Verkörperung mitprüfen. |
| NAK-16 | 08-19 | **ST-Map-Live-Refraktion** (User-Freigabe 19.08., ersetzt den NAK-15-Ansatz nach dessen Verwurf). **Pipeline recherche-gehärtet** — verbindlich ist `docs/research/2026-08-19-stmap-live-refraktion.md` (5-Worker-Deep-Research, Primärquellen, Blender-Fakten am installierten 5.1.2 verifiziert): Refraction BSDF statt Glass · 32f-EXR statt PNG-Hi/Lo · Box-Filter 0,01 + Denoise aus · Offset-Form → RG16F (LINEAR = WebGL2-Kern) · R/B-Normalisierung (B = Gewicht + TIR-Maske) · Lerp-Composite mit Glanz-Pass · Dispersion per 3-Tap. Erst EIN Probe-Still mit Probe-Checkliste beweisen (Maßstab: Glasinneres ändert sich mit dem Hintergrund — Ähnlichkeit zum alten Frame ist als Metrik VERBOTEN), dann 72er-Sequenz. Einstieg: `docs/NEXT-SESSION.md`. **Stand 19.08. (cba6bd6, 602b20a): Probe-Still GEBAUT und BESTANDEN** — 74,2 % der Glaspixel ändern sich mit dem Hintergrund (Messwerte `renders/stmap/messung.json`, Checkliste komplett beantwortet: B nicht Fresnel-gewichtet, 64 Samples genügen, Half-Float trägt, RGBA16F ok). Offen bleibt: User-Sichtung des lebenden Blatts (`stmap-probe.html`, Optik-Abnahme inkl. Dispersion-Stärke + Glanz-Licht-Rig), dann 72er-Sequenz (~18 min: ST-Maps 4 s + Glanz 11 s je Frame) und Integration in Prototyp/Schlieren-Blatt (dort den getContext-Patch für preserveDrawingBuffer mitnehmen). |
| NAK-17 | 08-19 | **Prisma-Mesh: Seitenflächen-Normalen zeigen nach INNEN** (Winding in `prisma-material-still.py` `from_pydata`; am evaluierten Depsgraph gemessen). Cycles rendert Glas trotzdem korrekt (Shading-Normalen werden geflippt) — alle abgenommenen Renders entstanden SO. Nicht nebenbei „fixen": ein Flip ändert potenziell MNEE-/Fasen-Nuancen aller Folge-Renders gegen den abgenommenen Stand. Nur zusammen mit einer bewussten Neu-Abnahme drehen. Bis dahin: Spiegelwinkel-/Normalen-Mathe immer am evaluierten Mesh messen, nie aus der Eckenliste herleiten. |

## Geschlossen

| ID | Geschlossen | Punkt |
|---|---|---|
| NAK-15 | 08-19, `7de964d` | **Prisma in der App ohne starren Hintergrund** (User-Anforderung 19.08.). Umgesetzt in der Nacht 19.08.: 72 RGBA-Drehframes `renders/dreh-frei/` (film_transparent, Plate+Boden kameraunsichtbar, Kamera identisch zur bewiesenen JS-Projektion) + 72 Boden-Kontaktframes `renders/dreh-boden/` (Prisma lichtwirksam aber unsichtbar, offline Alpha aus Helligkeit — Shadow Catcher war gemessen der falsche Weg, die Pfützen sind Glanzreflexe); Blatt komponiert transparent (clearRect, `?frames=dreh` Vergleich, Taste B/`?boden=0`), Pixel-Beweis: neben der Silhouette exakt Body-Farbe, Ebene 0 scheint durch. Alte Frames bleiben als Archiv/Prototyp-Quelle. **Nachtrag 19.08.: Ergebnis vom User VERWORFEN** („spiegelung des alten hintergrundes eingebacken … beleuchtung komplett falsch … billig zusammengeklebt") — die Silhouetten-Freistellung stimmt, aber das GLASINNERE trägt weiter die alte Konserve. Nachfolger: **NAK-16** (ST-Map-Live-Refraktion). |
