# 2026-09-02 — Wechsel in die visuelle Phase: Skizze in Figma, in der Sprache der Entwürfe

## Status

**Bindend, User-Wort vom 02.09.2026 (Laptop-Session).** Dieser Entscheid löst
den in `2026-08-31-technische-ui-architektur-arbeitsmodus.md` und im
Skizzen-Leseblatt angekündigten „ausdrücklichen Wechsel in die visuelle
Phase" ein. Er ändert nichts an der entschiedenen Funktions- und
Bedienarchitektur (Fünferblöcke 01 bis 05, Größenentscheid 01.09.2026); er
legt fest, **wo** und **in welcher Sprache** das visuelle Design entsteht.

## User-Wortlaut

Auftrag, mit Link auf die Figma-Datei `fable-dummy`:

> „lass uns mal besprechen wie wir das design in figma umsetzen. ich habe
> hier den link zu den ersten designs die du gemacht hast mit dem alten
> layout: https://www.figma.com/design/DvMbHg0MWCPwibDj8q6hI8/fable-dummy?node-id=0-1
> Da sind die alten größen layouts mit 700x noch, das neue wird ja 900x .
> als ersten schritt könntest du unsere aktuellen skizzen im design der
> entwürfe in figma umzusetzen. das visuelle design ist ja ganz gut
> gelungen. schau dich mal um und sag mir dann bevor du anfängst ob du
> verstanden hast was ich meine"

Nach der Rückmeldung, wie der Auftrag gelesen wird (Materialsprache der
Entwürfe auf das Layout der Skizze, drei Rahmen 1:1, neue Seite in
`fable-dummy`, Graphit-Schale, Arbeitsnamen bleiben):

> „ich möchte dass du das wording übernimmst. also bei Nakama Probeeq und
> Gen der blau rote verlauf"

„wording" wird als **Wortmarke** gelesen: die Schriftmarke `NAKAMA` /
`PROBEEQ` beziehungsweise `NAKAMA` / `GEN` mit dem rot-blauen Verlauf, die
der User schon am 22.08.2026 gegenüber der flachen Fassung eingefordert
hatte (`2026-08-21-gen-auftrag-figma.md`, Nachtrag 22.08.).

## Was damit festliegt

1. **Die visuelle Phase beginnt am 02.09.2026 in Figma.** Das Werkzeug ist
   die Figma-Datei **`fable-dummy`** (Key `DvMbHg0MWCPwibDj8q6hI8`), die der
   User beim Wechsel benannt hat (`design/LIES-MICH.md`, Regel 1: „welche
   Figma-Datei dann gilt, benennt der User beim Wechsel").
2. **Die Sprache ist die der Entwürfe vom 20. bis 22.08.2026** in dieser
   Datei: Unibody-Schale aus mattem Polycarbonat mit breitem Streiflicht
   und feinem Korn, eingelassene schwarze Glas-Messfläche mit Kantenglanz,
   Spiegelung und Vignette, ein Kinn aus Schalenmaterial mit Link-Apertur,
   Wortmarke, Schlüsselwort und flachen Einsatz-Tasten, Schrift Geist und
   Geist Mono, Label-über-Wert-Grammatik, weinrote Kurvenfüllung,
   nummerierte Band-Griffe, Zustände hohl, gefüllt oder gestrichelt, Cyan
   nur als Leuchtsaum und nie als Fläche (Materialstudie 01 in
   `fable-dummy`, Studie 03 Suna, Probeeq-Editor 700×420).
3. **Die Wortmarke mit dem rot-blauen Verlauf** kommt auf beide Apps:
   `NAKAMA` / `GEN` und `NAKAMA` / `PROBEEQ`. Sie wird aus dem bestehenden
   Knoten der Datei übernommen, nicht nachgebaut, damit der Verlauf
   (Shader-Füllung, rendert nur in Figma selbst) erhalten bleibt.
4. **Nur der Look wird übernommen, nichts von der alten Anordnung.** Die
   Layouts in `fable-dummy` (Editor 700×420, Suna-Kachel 260×84, Gen
   760×430) sind Verlauf. Layout, Hierarchie, Handgriffe und Zustände
   kommen ausschließlich aus der technischen Skizze
   `design/skizze/nakama-ui-technical-sketch.html` (Stand nach
   Fünferblock 05) und den datierten Abnahmen. Die Skizze bleibt die
   Layout-Wahrheit; Figma wird die visuelle Wahrheit.
5. **Größen:** Der User nannte „900x"; bindend bleibt der Größenentscheid
   vom 01.09.2026 (`2026-09-01-gen-nur-standardgroesse.md`): beide
   Gen-Flächen **950×538**, die Probeeq-Kachel nach Fünferblock 04/05
   **600×92** (Dichteprüfung, keine neue Größenabnahme).
6. **Erster Schritt: der Schönfall je Fläche, 1:1.** Gen Fläche 1, Gen
   Fläche 2 und Probeeq als je ein Rahmen im Vorschlagszustand der Skizze
   (Szenario `PROPOSAL`: offener Draft, Befund `READY TO SEND`,
   Transaktionszeile sichtbar), weil dieser Zustand die meisten Elemente
   der Grammatik zeigt und dem Draft-Kasten der alten Entwürfe entspricht.
   Zustände und Panels (Band-Panel, Global offen, Preset-Liste, Kopie,
   Link-Verlust, EQ aus) folgen als zweiter Schritt auf demselben Blatt.
7. **Tasten bleiben Material** (User-Gesetz 25.08.2026): Zustände wechseln
   Fläche, Schatten, Farbe oder Transform, nie die Maße; die Maße kommen
   aus der Skizze.

## Arbeitsannahmen (von Claude am 02.09.2026 vorgelegt, vom User nicht widersprochen)

- **Ort:** eine neue Seite in `fable-dummy` neben den alten Studien. Damit
  ist `fable-dummy` ab dem 02.09.2026 die Arbeitsdatei der visuellen Phase;
  die Depotregel vom 22./24.08.2026 („die sind IMMER in diesem figma" für
  `Nakama-Design`) gilt für diese Phase nicht weiter, solange der User
  keine andere Datei benennt.
- **Schale:** Graphit (dunkel zuerst, User-Wort 22.08.2026 „stand jetzt aber
  erstmal nur dunkel"). Die neuere Silber-Schale mit dunklem Glas liegt in
  der Datei und kommt nur auf Zuruf.
- **Namen:** `OVERVIEW` und `EQ` im Kopf bleiben Arbeitsnamen bis zum
  Entscheid der Karte U23.
- **Übersetzungsregel:** Kopfzeile mit Wortmarke, Umschalter und
  Session-Zeile als Schalenband oben; Arbeitsfläche im Glas; die untere
  Zeile von Gen Fläche 2 als Kinn mit Einsatz-Tasten; die Probeeq-Kachel mit
  Zeile 1 als Schalenband (Apertur, Name, Bus, Tasten) und Zeile 2 als Glas
  (Messwahrheit). Neue Elemente ohne altes Vorbild (Sources-Spalte, Befund,
  Zielleiste) folgen derselben Grammatik.

## Was unverändert gilt

- Die Skizze wird weiter nach jedem Fünferblock fortgeschrieben und bleibt
  die sichtbare Funktionswahrheit; ihre Werkzeugfarben sind keine Vorgabe.
- Bindend bleiben nur datierte Abnahmen. Ein Figma-Stand ist ein Vorschlag,
  bis der User ihn ausdrücklich abnimmt; die Spiegelung nach `eq-copilot/`
  beginnt erst danach (Design-Arbeitsmodell Punkt 4 in `CLAUDE.md`).
- Blueprint Abschnitt 15, Punkt 11 bleibt offen, bis Maße, Typografie und
  Renderabnahme aus dieser Phase datiert abgenommen sind.

## Belege (nachgetragen am 02.09.2026)

Die drei Rahmen liegen in `fable-dummy` auf der neuen Seite „01 — Nakama
950×538 · Skizze in Materialsprache" (Node `6116:2`): Gen Fläche 1 Unibody
`6116:6` auf Bühne `6116:3`, Gen Fläche 2 `6116:11` auf `6116:8`, Probeeq
`6116:16` auf `6116:13`. Exporte unter `design/visuell/belege/`
(`2026-09-02-v01-gen-flaeche-1-overview@2x.png`,
`2026-09-02-v01-gen-flaeche-2-eq@2x.png`,
`2026-09-02-v01-probeeq-kachel@2x.png`, Bildschirmfotos der zweifach
skalierten Bühne); Leseblatt mit Knoten,
Übersetzungsregeln und offenen Punkten: `design/visuell/LIES-MICH.md`.
Der Stand ist ein Vorschlag von Claude und nicht abgenommen.

**Zweiter Schritt, 02.09.2026 (Auftrag „mach jetzt die zustände und panels
als zweiten schritt"):** 21 Zustandsrahmen als Klone der Grundrahmen auf
derselben Seite (Befundliste, Quelle ohne Befund, Filter, Draft offen,
Kandidat und Link-Verlust für Fläche 1; Band-Panel in vier Ansichten, Global,
Preset-Liste, Kopie, Halten, Kandidat, Bestätigt und Link-Verlust für
Fläche 2; EQ aus, Link-Verlust, Kandidat und Bestätigt für Probeeq). Belege
`2026-09-02-v02-*@2x.png` und die Tabelle mit Knoten und den bewusst
benannten Abweichungen stehen in `design/visuell/LIES-MICH.md`.

## Sessionende 02.09.2026 (Laptop, Nachmittag)

Geliefert und gepusht: Seite „01 — Nakama 950×538 · Skizze in
Materialsprache" in `fable-dummy` mit drei Grundrahmen (Commit `ff82162`)
und 21 Zustandsrahmen (Commits `1eb9012`, `b987fb4`), Belege in
`design/visuell/belege/`, Leseblatt `design/visuell/LIES-MICH.md`, Nachträge
in CLAUDE.md, beiden Leseblättern und Blueprint 15.11.

Offen geblieben, alles Entscheide des Users:

- Die Abnahme des Figma-Stands (Grundrahmen und Zustände) oder
  Änderungswünsche daran; bis dahin bleibt alles Vorschlag.
- Graphit oder die neuere Silber-Schale; gebaut ist Graphit.
- Die Produktnamen der beiden Gen-Flächen (Karte U23; `OVERVIEW` und `EQ`
  sind Arbeitsnamen).
- Aus Fünferblock 05 weiterhin zur Bestätigung: Probeeq-Zeile 2 immer
  sichtbar bei fester Höhe und die Kachelbreite 600 px; beides ist in Figma
  so gebaut.
- Die flüchtigen Zustände der Skizze ohne Rahmen (Überschreiben-Bestätigung,
  Namensfeld bei `SAVE AS`, Validierungszeile, Graph-Hinweise, Hover).

Nächster sinnvoller Schritt: Der User sieht sich die Seite in Figma an
(dort rendert die Wortmarke mit Verlauf) und gibt Abnahme oder Änderungen;
danach Skalierungsstufen und gebackene Assets je Stufe oder ein dritter
Schritt mit den flüchtigen Zuständen.

## Poliersession 02.09.2026 (Laptop, Abend)

User-Wortlaut zum Auftrag:

> „die designs von letzter session brauchen noch eine ausführliche
> poliersession. da fehlt der feinschliff an ganz vielen stellen, die den
> unterscheid zwischen profi design und "netter versuch" machen."

Nach der vorgelegten Lesart (echte Fehler zuerst: Log-Frequenzachse,
unglaubwürdige Kurven, harte Kante der Weinfüllung, Durchstreich-Griff,
unbündige dB-Skalen, Halbpixel, drei Zahlenstände für einen Draft; dann
Feinschliff: Kantenraster 22/928, Aktionsblock Fläche 1 als drei Pillen à 30,
Palette und Typo straffen, Befundzone beruhigen, Kurvenschlüssel auf beiden
Flächen, Probeeq-Zeile 1 zentrieren, Filterfeld als Einlass; nicht angefasst:
Materialrezept, Skizzenmaße und Tastenhöhen, MASTER-Ring, Wortmarke):

> „ja mach das so erstmal"

Der Stand nach der Poliersession bleibt ein Vorschlag von Claude; die
Ergebnisse und die bewusst benannten Abweichungen stehen in
`design/visuell/LIES-MICH.md` (Abschnitt „Poliersession").

**Belege (nachgetragen):** alle 24 Rahmen derselben Seite sind in Figma
poliert (Knoten unverändert, Inhalte neu gebaut); Bildschirmfotos
`design/visuell/belege/2026-09-02-v03-*@2x.png` neben den Ständen `v01`
und `v02` von vor der Poliersession.
