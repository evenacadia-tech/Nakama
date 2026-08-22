# 2026-08-20 — Main bekommt Overview + Detail

**Abnahme, nicht Arbeitsannahme.** Beurteilt wurde das, was tatsächlich zur
Debatte stand: fünf Disclosure-Mechaniken auf der abgenommenen Größe
**760×430**, in echten Pixeln, in reinen Graustufen, mit ehrlichen Blättern
(keine Liste behauptet mehr, als sie zeigt) und durchschaltbar über alle drei
Arbeitsschritte. Damit ist die Voraussetzung aus [LIES-MICH.md](LIES-MICH.md)
erfüllt.

## Das wörtliche Urteil

Gefragt wurde visuell, nicht verbal — deshalb ist das Protokoll des Werkzeugs
das Zitat ([methode-visuelle-befragung](2026-08-20-methode-visuelle-befragung.md)):

```json
{
 "werkzeug": "nakama-wireframe-main", "version": 2,
 "zeitpunkt": "2026-08-20T17:12:00.977Z",
 "stufe": "2 / Schritt 1 — Wireframe UX", "oberflaeche": "Main",
 "engereWahl": ["uebersicht", "fokus"],
 "sieger": "uebersicht",
 "mechanik": "Overview + Detail",
 "name": "Kleine Karte bleibt, Arbeitsfläche wechselt",
 "ablehnungen": [],
 "sekunden": { "schritt1": "256.8", "schritt2": "48.4" }
}
```

Vier Minuten an den fünf Kacheln, danach 48 Sekunden im Zweikampf. Kein
Schnellschuss, und keine Ablehnung notiert.

## Was damit festliegt

**Main arbeitet nach Overview + Detail:** oben eine dauerhafte kleine Karte
des Mixes ohne Namen, die zeigt, wo man gerade steht; darunter eine
Arbeitsfläche, die mit der Aufgabe wechselt.

Der **Dauerhaft-Vertrag** kommt damit mit — er wurde nicht behauptet, sondern
gemessen (dauerhaft = steht in *allen drei* Arbeitsschritten auf dem Blatt):

| | Baustein | vom Mix |
|---|---|---|
| **dauerhaft** | Bedienebenen + Status | — |
| **dauerhaft** | Übersichtskarte ohne Namen | **11 von 16** |
| einen Schritt entfernt | Befundliste · Quellenliste mit Namen · Assistent · Detail einer Quelle · A/B-Urteil | |

## Das Muster hinter der Wahl

Es lohnt sich, es festzuhalten, weil es die Wahl überlebt, falls die
Ausführung später geändert wird:

**Beide Finalisten waren die einzigen zwei Mechaniken, die überhaupt etwas vom
Mix dauerhaft stehen lassen** — Overview + Detail 11 von 16 (als Ort, ohne
Namen), Focus + Context 7 von 16 (lesbar, mit Namen). Die drei Mechaniken mit
**0 von 16** (Schritt für Schritt · Nur auf Abruf · Semantischer Zoom) sind
schon in der ersten Runde ausgeschieden.

Der Mix soll beim Arbeiten **sichtbar bleiben**. Zwischen den beiden Wegen
dorthin hat der Vollständigkeit (11 Quellen als Ort) den Vorzug vor
Lesbarkeit (7 Quellen mit Namen) bekommen.

## Was aus dem Verlierer mitzunehmen ist

Focus + Context konnte etwas, das der Sieger nicht kann: **Namen**. Seine
sieben Zeilen waren lesbar, die elf des Siegers sind nur Positionen. Wo in
Overview + Detail eine Quelle benannt werden muss, ist das eine offene
Aufgabe der Arbeitsfläche, nicht der Karte.

## Was ausdrücklich offen bleibt

- **Was die Karte zeigt** und was mit den **fünf Quellen** geschieht, die
  nicht hineinpassen (16 sind laut Spezifikation möglich). Gewählt wurde die
  Mechanik, nicht der Inhalt der Karte.
- **Der Inhalt der Arbeitsfläche je Schritt.** Im Werkzeug ist er ein
  Entwurf von Claude, kein Urteil des Users — im Schritt „Mix lesen" standen
  dort gemessen nur `Befunde 2 von 3` und `Quellen 3 von 16`. Das ist eng
  und muss gelöst werden.
- **Die vier Bedienebenen**: „sichtbar getrennt" ist bisher ein Streifen mit
  vier Wörtern. Was die Trennung *bedeutet*, ist ungestaltet.
- Zustände: frisch / veraltet / getrennt, Messung läuft / unvollständig,
  degradierte Funktion — als Schalter noch nicht gebaut.
- Alles Visuelle. Graustufen bleiben Graustufen, bis Stufe 2 durch ist.

## Kein Prüfpunkt ausgelöst

Welt und Fassung (Glas und Licht · dunkles Glas, ein Lichtleiter) bleiben
**Arbeitsannahmen**. Ihr Prüfpunkt lautet „sobald das Layout von Main steht" —
das Layout steht **nicht**: entschieden ist die Mechanik, nicht die Anordnung,
nicht die Zustände, und Stufe 2 Schritt 2 und 3 stehen noch aus. Claude darf
sie weiterhin nicht als entschieden zitieren.

## Bezug

- Werkzeug und gemessener Stand: `werkzeug/LIES-MICH.md`, Abschnitt
  `wireframe-main.html`
- [groessen-alle-drei](2026-08-20-groessen-alle-drei.md) — die 760×430,
  auf denen geurteilt wurde
- [sessionende](2026-08-20-sessionende.md) — enthält die überholte
  Messtabelle mit Korrekturvermerk
