# 2026-08-21 — Sessionende: Main steht, der Editor ist dran

Diese Datei ist der einzige Wissenstransfer. Was hier nicht steht, ist weg.

## Wo wir stehen

**Phase 1 für Main abgeschlossen.** Das Wireframe von Main ist auf allen
Ebenen abgenommen — **fünf** Urteile am 1:1-Blatt auf 760×430 (Mechanik ·
Karte + Aufteilung · Vorhören · Zustände — dazu die Bedienebenen-Runde),
plus die Größenabnahme am nachgestellten FL-Bildschirm. Alle in `abnahmen/`.

> **⚠ Berichtigt 2026-08-21:** hier stand „sieben Urteile, alle am 1:1-Blatt
> auf 760×430 gefällt“. Zwei Zeilen der Tabelle darunter sind das nicht: die
> Größen wurden am Bild mit allen drei Oberflächen auf dem nachgestellten
> FL-Bildschirm beurteilt (760×430 war das Ergebnis, nicht die Bühne), und
> Welt/Fassung sind ausdrücklich **keine** Urteile dieser Ebene, sondern
> Arbeitsannahmen von einem 448×310-Ausschnitt.

| Was | Entscheidung | Datei |
|---|---|---|
| Disclosure-Mechanik | **Overview + Detail** | [mechanik-main-overview-detail](2026-08-20-mechanik-main-overview-detail.md) |
| Inhalt der Karte | **jede Quelle, eine Zeile** | [karte-alle-quellen](2026-08-20-karte-alle-quellen.md) |
| Aufteilung | **Überschuss teilen, Mangel nicht** | dieselbe Datei |
| Vorhören | **markierte Zeile**, keine App-Umrandung | [vorhoeren-markierte-zeile](2026-08-20-vorhoeren-markierte-zeile.md) |
| Zustände | **nur die Ausnahme spricht** | [zustaende-nur-ausnahme](2026-08-20-zustaende-nur-ausnahme.md) |
| Größen | Main 760×430 · Active 700×420 · Kachel 260×84 | [groessen-alle-drei](2026-08-20-groessen-alle-drei.md) |
| Welt / Fassung | **⚠ Arbeitsannahmen**, nicht abgenommen | [welt-glas-und-licht](2026-08-20-welt-glas-und-licht.md) · [fassung-lichtleiter](2026-08-20-fassung-lichtleiter.md) |

## Der Entwurf von Main, an einem Ort

Damit die nächste Session ihn nicht aus sieben Dateien zusammensetzen muss:

**Dauerhaft sichtbar** (28 px Streifen + Karte):
- **Streifen**: vier Bedienebenen als Reiter, der aktive leuchtet · Statuszeile
  (nur Abweichung, mit Namen solange es ≤2 je Sorte sind; sonst „All N fresh") ·
  Messung · degradierte Funktion.
- **Karte**: eine Zeile je Quelle, **ohne Namen**, alle gleich hoch.
  Zellen: gefüllt = frisch · **hohl = veraltet** · **Strichlinie = getrennt**.
  Keine Zeile ändert je ihre Höhe.

**Wechselt mit dem Arbeitsschritt** (Arbeitsfläche darunter):
1. *Mix lesen* → Befundliste, einer offen, der Rest als benannte Zeile.
2. *Befund verstehen* → Assistent in der vorgeschriebenen Fünf-Teile-Form.
3. *Urteilen* → A/B-Vergleich + Detail einer Quelle.

**Aufteilung**: Karte und Arbeitsfläche bekommen erst ihre natürliche Höhe,
der **Überschuss wird geteilt** (`flex:1 1 auto`, nicht `1fr 1fr`). Bei
Mangel hält die Karte ihre natürliche Höhe, die Arbeitsfläche gibt nach.

**Vorhören**: `Audition (hold)` wirklich halten. Zeichen = **markierte Zeile
in der Karte** + gedrückter Griff + leuchtender `AUDITION`-Reiter. Loslassen
= sofort weg, kein Rest. **Angewandt** = Marke bleibt an der Quelle +
„1 change applied" im Streifen + Rückweg.

Gemessene Eckwerte bei 16 Quellen: Streifen 28 px, für Karte + Arbeit bleiben
**368**. Karte 212, Arbeit 156. Assistent 154 (bei 156 verfügbaren — knapp).

> **⚠ Berichtigt 2026-08-21:** hier stand **370**. Nachgerechnet und am Blatt
> nachgemessen sind es **368** — 428 (clientHeight) − 16 (Polster) − 28
> (Streifen) − 16 (zwei Lücken). Der 1-px-Rahmen oben und unten war nicht
> abgezogen. Derselbe Fehler steht in
> `abnahmen/2026-08-20-karte-alle-quellen.md`. Die übrigen Zahlen stimmen auf
> das Pixel. Zusatz: 212/156 gelten für den Schritt „Befund verstehen“ — in
> Schritt 1 sind es 216/152, in Schritt 3 232/137, weil der Überschuss dort
> anders anfällt.

## Der nächste Schritt, konkret

**Active-Probe-Editor, 700×420.** Reihenfolge laut
`docs/CLAUDE-UX-UI-ARBEITSKERN.md` und dem Muster dieser Session:

1. **Aufgabenliste nach Häufigkeit** schreiben — ohne sie kein Wireframe
   (`docs/DESIGN-GESETZE.md`, verbindliche Folge 1).
2. **Natürliche Höhen messen**, bevor irgendetwas angeordnet wird. Bei Main
   war das die entscheidende Runde: erst die Klemme kennen, dann entwerfen.
3. Erst dann Varianten, und die unterscheiden sich in der
   **Disclosure-Mechanik**, nicht in der Kastenanordnung.

Die Klemme ist dort absehbar schärfer als bei Main: 700×420 ist **kleiner**
als Main, und der Inhalt ist reicher — EQ-Kurve als zentrale Anzeige, **acht
Bänder mit je zwölf Parametern** (an/aus, Typ, Frequenz, Q, Gain, Kanalmodus,
dynamisch an/aus, Schwelle, Bereich, Attack, Hold, Release), Global-Trims, Bypass,
Draft/Preview vom Main (ferngesteuert enger begrenzt: ±3 dB statt ±12),
Undo-Verlauf, Schutzbereiche, Statuszeile mit Pairing.

Acht Bänder × zwölf Parameter sind **96** Werte, dazu fünf globale
(In-Trim, Out-Trim, Stereobreite, Mono-Bass, Bypass) — zusammen **101**
einstellbare Werte auf 700×420. Die erste Messung muss lauten:
**was kostet eine Bandzeile, und wieviele passen neben die Kurve?**

> **⚠ Berichtigt 2026-08-21:** hier stand „zehn Parameter … achtzig Werte“,
> während die Klammer eine Zeile darüber zwölf aufzählte. Nachgezählt an
> `docs/oberflaechen-spezifikation.md`, Abschnitt 2: an/aus (1) · Typ (2) ·
> Frequenz (3) · Q (4) · Gain (5) · Kanalmodus (6) · dynamisch an/aus (7) ·
> Schwelle (8) · Bereich (9) · Attack (10) · Hold (11) · Release (12).
> Der Inhalt ist also **20 % größer** als angesetzt — auf einer Fläche, die
> 10 px weniger Höhe und 60 px weniger Breite hat als die bereits knappe von
> Main. Die vorgerechnete Klemme steht in `docs/sondenplan.md`.

## Werkzeuge: was aktuell ist und was Beleg ist

| Datei | Stand |
|---|---|
| `werkzeug/zustaende.html` | **aktuellster Main-Entwurf**, hier weiterarbeiten |
| `werkzeug/bedienebenen.html` | Beleg der Bedienebenen-Wahl, eingefroren |
| `werkzeug/wireframe-main-karte.html` | Beleg der Kartenwahl, eingefroren |
| `werkzeug/wireframe-main.html` | Beleg der Mechanikwahl, eingefroren |
| `werkzeug/PRUEFLISTE.md` | **neu** — was an jedem Blatt gemessen wird, mit dem Fehler dahinter |
| `werkzeug/licht.html` | **stillgelegt**, Modell falsch, vor Gebrauch neu bauen |
| `werkzeug/stilbefragung.html` | zeigt noch die verworfene Welt, vor Gebrauch umstellen |

Die eingefrorenen Blätter **nicht** nachziehen: sie belegen, was zur Wahl
stand. Wo ein Blatt heute etwas anderes zeigt als beim Urteil, steht das im
jeweiligen Abnahme-Text.

## Vier Fallen, in die ich in dieser Session getappt bin

1. **Sekundärquellen geglaubt.** Die Messtabelle im letzten Sessionende war
   in 6 von 15 Feldern falsch, und ihre Diagnose stimmte für keine der drei
   betroffenen Mechaniken. Hätte ich sie übernommen, hätte der User auf einer
   erfundenen Grundlage entschieden. **Nachmessen, immer.**
2. **Das Urteil falsch übersetzt.** „1 und 2 eine kombination daraus" habe
   ich als Streifen + Blattrahmen gelesen und gebaut; gemeint waren markierte
   Zeile + App-Umrandung. Der Korrekturvermerk steht in
   [bedienebenen-beides](2026-08-20-bedienebenen-beides.md). **Bei
   Nummernverweisen zurückfragen, was gemeint ist.**
3. **Eigene Prüfungen blind werden lassen.** Nach einem Umbau griff ein
   Prüfmuster ins Leere und meldete „heil". Ein Gate, das nicht scheitern
   kann, beweist nichts — **jede Prüfung mit einem künstlichen Fehler
   gegenprüfen.**
4. **Beim Aufräumen zu viel mitgenommen.** Eine Dead-Code-Löschung hat neu
   eingebaute Funktionen mitgerissen; gefunden nur, weil das Blatt danach
   beim Zeichnen abstürzte. **Nach jedem Aufräumen die Syntax prüfen und das
   Blatt einmal zeichnen lassen.**

## Was sich als Arbeitsmuster bewährt hat

- **Erst messen, dann entwerfen.** Jede Runde begann mit natürlichen Höhen
  und Breiten. Die Entscheidungen wurden dadurch klein und begründbar.
- **Der Preis gehört auf das Blatt.** Wo eine Variante etwas verliert, zeigt
  sie es (`3/5`, „2 more not shown"). Der User hat zweimal mit sichtbarem
  Preis gewählt.
- **Ein genannter Preis ist eine Aufgabe, kein akzeptierter Mangel.** Nach
  der Kartenwahl und nach der Zustandswahl wurde der Preis eingelöst, ohne
  die Wahl anzutasten.
- **Grenzfälle zuerst prüfen.** Der Schönfall sah bei allen Zustandsvarianten
  tadellos aus; „alles getrennt" hat drei Lügen freigelegt.

## Offen und ungefragt

- **Wie kommt man von einer hohlen Zeile zur Quelle?** Die Karte ist
  namenlos. Der Name steht im Streifen, solange es wenige sind — sonst in
  der Quellenliste, einen Schritt entfernt. Ob das reicht, zeigt der Gebrauch.
- Braucht **veraltet** eine Abstufung (seit 10 s / seit 10 min)?
- **Mehrere angewandte Änderungen**: „1 change applied" ist ein Zähler ohne
  Verlauf. Die Spezifikation kennt einen Undo-Verlauf nur für den Editor.
- Ist **760×430** wirklich die kleinste angebotene Stufe? (75 % schneidet ab.)
- Braucht die **Passive-Kachel** bei vielen Instanzen eine kleinere Stufe?
- **Stufe 2 Schritt 2 „Early Visual Layer"** ist nicht begonnen.

## Prüfpunkt der Arbeitsannahmen: noch nicht erreicht

Welt (Glas und Licht) und Fassung (dunkles Glas, ein Lichtleiter) bleiben
**Arbeitsannahmen**. Ihr Prüfpunkt lautet „sobald das Layout steht". Main
steht — aber zwei Oberflächen fehlen und Stufe 2 Schritt 2 ist nicht
begonnen. **Claude darf sie weiterhin nicht als entschieden zitieren.**

## Nachtrag 2026-08-21, 21:09 — Designvertrag und Figma

**Was heute danach passiert ist:** Der User hat im **Kontext-Interview**
(zwanzig Fragen, in der Technik-Session) die Wahrheit festgelegt; für dieses
Repo steht sie mit Wortlaut in
[2026-08-21-designvertrag](2026-08-21-designvertrag.md): drei Apps mit einer
Identität — **Nakama Gen** (Main) · **Nakama Probeeq** (aktive Sonde, ein
vollwertiger EQ, der Anweisungen von Gen umsetzt und manuell bedienbar ist) ·
**Nakama Suna** (passive Sonde); **Figma ist die Quelle, das Repo übersetzt**;
Produkt englisch („Englisch – mein Wort"); Glas und Licht **verworfen**
(Vermerk oben in beiden Arbeitsannahmen — der Prüfpunkt aus dem letzten
Abschnitt dieser Datei ist damit eingelöst); Startwerte hingenommen. Die
beiden Figma-Stände liegen bytegleich in `assets/figma/` (Gerätekörper
gemessen: genau 4 × 760×430 und 4 × 700×420). Die Schleuse vor `prototyp/`
ist **offen** (gemessen). `CLAUDE.md`, `docs/arbeitsplan.md` (Phase 0 ✓,
neue Phase 1b), `docs/oberflaechen-spezifikation.md`, `docs/sondenplan.md`,
`werkzeug/LIES-MICH.md` und der Primer sind nachgezogen.

**Als Nächstes:** Phase 1b, Schritt Gen — den Figma-Stand
`assets/figma/2026-08-21-gen.png` Element für Element gegen Spezifikation
und Abnahmen lesen (drei Listen: deckt sich · weicht ab · fehlt im Bild;
Anfang in `Nakama Designausarbeitungen selfmade/LIES-MICH.md`), dann das
Blatt 1:1 auf 760×430 in `prototyp/` bauen — vorher mit dem User: ein Blatt
je Oberfläche oder Tabs. Offen und nur vom User zu klären: die drei
Zeichen, in denen der Stand von den Abnahmen abweicht (STALE gestrichelt
statt hohl · `1 STALE` ohne Namen · Marker in Ruhe auf der Fokusquelle);
zwölf oder dreizehn Parameter je Band (NAK-33, Technik-Repo); ein
Figma-Stand für Suna fehlt noch.
