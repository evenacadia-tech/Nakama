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
| NAK-15 | 08-19 | **Prisma muss in der App FREIGESTELLT sein (User-Anforderung 19.08.: „später in der app muss der prisma aber ohne starren hintergrund dann sein"):** Die 72 Drehframes (`prisma/renders/dreh/f*.webp`) sind lossy VP8 OHNE Alphakanal (geprüft am Chunk-Header) — Studio-Hintergrund eingebacken. Für die App: Drehsequenz mit Cycles `film_transparent` neu rendern (CPU, Arc-Landmine); Boden-Lichtpfützen/Schatten entweder weglassen (Prisma schwebt frei — User-Entscheid) oder als Shadow-Catcher-Ebene mitkomponieren (Blender-Handbuch verifiziert: Shadow-Catcher-Objekte erfassen mit aktiviertem Shadow-Catcher-Pass ALLE indirekten Licht-Interaktionen, auch Aufhellung). Blatt-Compositing braucht dann: `clearRect` vor dem Frame-Draw (sonst Geisterbilder bei Alpha-Frames), `#bild`-CSS-Hintergrund transparent, WebP MIT Alpha (VP8X/ALPH) oder PNG. Ebene 0 (Winter Nexus) scheint dann DURCH das Blatt hindurch statt es nur zu rahmen. |

## Geschlossen

_(noch leer — Schließungen hier mit ID + SHA eintragen, nicht löschen)_
