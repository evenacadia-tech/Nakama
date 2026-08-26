---
id: w-2026-08-26-plugin-modernisierung-ohne-vertrauensbruch
typ: wissenseintrag
status: aktiv
datum: 2026-08-26
aktualisiert: 2026-08-26
hauptbereich: fl-studio
weitere_bereiche: [engineering, ux, ui-motion]
pruefstand: quellen-geprueft
---

# Plugin-Modernisierung ohne Vertrauensbruch

## Menschliches Warum

Menschen öffnen alte Projekte nicht, um eine Migration zu testen, sondern um
an ihrer Musik weiterzuarbeiten. Sie erwarten, dass Klang, Presets,
Automation, gespeicherter Zustand und eingeübte Abläufe weiterhin stimmen.
Eine moderne Oberfläche ist deshalb nur ein Fortschritt, wenn sie dieses
Vertrauen nicht gegen technische Sauberkeit eintauscht.

Die Kausalkette lautet:

> Weil Produzenten auf wiederaufrufbare Projekte und erlernte Bedienung
> angewiesen sind, braucht eine Plugin-Modernisierung zuerst eingefrorene
> Außenverträge und Vergleichsbelege, damit neue Technik und Gestaltung keine
> unbemerkte Klang-, State- oder Workflow-Änderung erzeugen.

## Belegtes Wissen

Der Praxisbericht aus der FL-Studio-Plugin-Suite empfiehlt keine komplette
Neuschreibung. Bestehender Code bleibt oft gerade deshalb lange erhalten, weil
er noch Nutzerwert liefert. Modernisierung soll schrittweise erfolgen und
externes Verhalten während eines Refactorings unverändert lassen.

Wenn die ursprüngliche Absicht unklar ist, nennt der Vortrag mehrere
Beweisquellen: veröffentlichte Binärdateien als Referenz, Versionsgeschichte,
Nutzerdokumentation, Backlog, Tests, Testprojekte, Presets, Teamwissen,
Compilerwarnungen, Logs sowie statische und dynamische Analyse. Vor dem Umbau
müssen die Zielinvarianten ausdrücklich feststehen: insbesondere Klangtreue,
Rückwärtskompatibilität und erlaubte neue Funktionen.

Die empfohlene Reihenfolge ist:

1. Baseline einfrieren, Werkzeuge aktualisieren und Build prüfen.
2. Reproduzierbaren Ein-Befehl-Build, CI, Tests, Nightlies und Validierung
   herstellen; dann in kleinen, jederzeit baubaren Änderungen aufräumen.
3. Erst auf dieser Grundlage GUI und neue Funktionen ersetzen.

Bei einer Vektor-GUI-Modernisierung trennt der Vortrag Editor und Engine,
warnt vor stillen Änderungen an Parametertypen oder Wertebereichen und rät,
die alte Oberfläche so lange als visuelle und funktionale Referenz verfügbar
zu halten, bis die neue verlässlich geprüft ist. Auch Build-Artefakte sollen
für spätere Vergleiche archiviert werden.

## Konsequenz für Entwurf und Bedienung

- Eine visuelle Neuordnung darf keine Parametersemantik, Presetbedeutung oder
  Hostautomation nebenbei verändern.
- Der alte und der neue Pfad brauchen identische Testfälle für Laden,
  Speichern, Automation, Undo, Bypass, mehrere Instanzen und Klang.
- Kleine, in sich baubare Änderungen machen Abweichungen lokalisierbar. Ein
  großer GUI-/Engine-Umbau würde Ursache und Wirkung vermischen.
- Referenzoberflächen dienen nicht als Stilgrenze. Sie zeigen, welche Zustände
  und Handlungen bisher erreichbar waren und ob die neue Oberfläche etwas
  verliert oder umdeutet.
- KI-generierte Änderungen sind nur so belastbar wie die unabhängigen Tests
  und das menschliche Verständnis ihres Außenverhaltens.

## Visuelle Belege

- [12:17 – „Where are we?“](https://www.youtube.com/watch?v=zY8uHzAdnzk&t=737s):
  ordnet Binärreferenz, Historie, Dokumentation, Presets und Tests als
  Beweisquellen ein.
- [17:25 – Modernisierungsphasen](https://www.youtube.com/watch?v=zY8uHzAdnzk&t=1045s):
  zeigt Build, CI und Tests vor der neuen GUI.
- [32:55 – Checkliste zur Vektor-GUI](https://www.youtube.com/watch?v=zY8uHzAdnzk&t=1975s):
  verbindet Editor-/Engine-Trennung, Parameterstabilität, Presetmigration und
  den alten GUI-Referenzpfad.

## Bedeutung für Nakama

Der Web-Simulator ist als Nakamas ausführbare Referenz für neue Zustände,
Gesten und Motion vorgesehen, sobald ein benannter Release-Candidate visuell
abgenommen wurde; freigegebene PNGs sind die visuelle Referenz. Beim späteren
nativen Transfer dürfen diese neuen Referenzen den bestehenden technischen
Außenvertrag nicht still verändern. Parameter, Projekt-State, Presets,
Automation und Audioverhalten werden deshalb getrennt vom UI-Umbau bewiesen.

Als Nakama-Arbeitshypothese sollten der freigegebene Web-Release-Candidate und
der bisherige native Editor als Vergleichspfade verfügbar bleiben, bis die
neue JUCE-Oberfläche im echten FL-Studio-Lebenslauf bestanden hat. Das heißt
nicht, das alte Design dauerhaft weiterzuführen; es bewahrt einen prüfbaren
Rückweg während der Übertragung.

## Grenzen, Widersprüche und offene Fragen

Der Vortrag ist ein Erfahrungsbericht aus Image-Lines eigener Suite, keine
allgemeine Migrationsnorm. Seine Plugins, Frameworks und historischen
Randbedingungen sind nicht identisch mit Nakama. Die Empfehlung, eine alte UI
vorübergehend als Referenz zu behalten, ist daher ein Prüfprinzip und keine
verbindliche Produktentscheidung. Das ausgewertete Transkript war automatisch
erzeugt; sichtbare Kernaussagen wurden am Video gegengeprüft.

## Quellen

- Titel: *Modernizing Legacy Audio Plugin Codebases – Lessons from FL Studio’s Plugin Suite*
- Autor: Tomas Medek
- Organisation: Image-Line
- Jahr: 2025
- URL: https://www.youtube.com/watch?v=zY8uHzAdnzk
- Eingang: `youtube-transkript`
- geprüft: vollständiges Auto-Transkript, Videobeschreibung und ausgewählte
  Folien im Video

## Verknüpfungen

- [`../../UX-CONTRACT.md`](../../UX-CONTRACT.md)
- [`../../DESIGN.md`](../../DESIGN.md)
- [`../../docs/PLAN-STAND.md`](../../docs/PLAN-STAND.md)
- [`../../design/reviews/2026-08-25-audio-plugin-ux-analyse.md`](../../design/reviews/2026-08-25-audio-plugin-ux-analyse.md)
- [`../engineering/2026-08-26-audiodevcon-videoauswahl.md`](../engineering/2026-08-26-audiodevcon-videoauswahl.md)
