# 2026-08-21 — Sessionende: Main steht, der Editor ist dran

Diese Datei ist der einzige Wissenstransfer. Was hier nicht steht, ist weg.

## Wo wir stehen

**Phase 1 für Main abgeschlossen.** Das Wireframe von Main ist auf allen
Ebenen abgenommen — sieben Urteile, alle am 1:1-Blatt auf 760×430 gefällt,
alle in `abnahmen/`.

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
370. Karte 212, Arbeit 156. Assistent 154 (bei 156 verfügbaren — knapp).

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
Bänder mit je zehn Parametern** (an/aus, Typ, Frequenz, Q, Gain, Kanalmodus,
dynamisch mit Schwelle/Bereich/Attack/Hold/Release), Global-Trims, Bypass,
Draft/Preview vom Main (ferngesteuert enger begrenzt: ±3 dB statt ±12),
Undo-Verlauf, Schutzbereiche, Statuszeile mit Pairing.

Acht Bänder × zehn Parameter sind achtzig Werte. Die erste Messung muss
lauten: **was kostet eine Bandzeile, und wieviele passen neben die Kurve?**

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
