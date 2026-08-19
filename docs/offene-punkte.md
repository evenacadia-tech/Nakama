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

## Geschlossen

| ID | Geschlossen | Punkt |
|---|---|---|
| NAK-15 | 08-19, `7de964d` | **Prisma in der App ohne starren Hintergrund** (User-Anforderung 19.08.). Umgesetzt in der Nacht 19.08.: 72 RGBA-Drehframes `renders/dreh-frei/` (film_transparent, Plate+Boden kameraunsichtbar, Kamera identisch zur bewiesenen JS-Projektion) + 72 Boden-Kontaktframes `renders/dreh-boden/` (Prisma lichtwirksam aber unsichtbar, offline Alpha aus Helligkeit — Shadow Catcher war gemessen der falsche Weg, die Pfützen sind Glanzreflexe); Blatt komponiert transparent (clearRect, `?frames=dreh` Vergleich, Taste B/`?boden=0`), Pixel-Beweis: neben der Silhouette exakt Body-Farbe, Ebene 0 scheint durch. Alte Frames bleiben als Archiv/Prototyp-Quelle. Die JUCE-App-Integration selbst läuft unter NAK-02; Optik-Abnahme am Blatt beim User. |
