# 2026-08-20 — Sessionende: Stand und nächster Schritt

Diese Datei ist der einzige Wissenstransfer. Was hier nicht steht, ist weg.

## Wo wir stehen

**Phase 0 abgeschlossen** (Welt und Fassung), **Phase 0.5 abgeschlossen**
(Formfaktor und Größe). **Phase 1 begonnen**, aber nicht entschieden.

| Was | Entscheidung | Datei |
|---|---|---|
| Methode | visuell befragen, auf Weltebene beginnen | [methode-visuelle-befragung](2026-08-20-methode-visuelle-befragung.md) |
| Welt | Glas und Licht · Place · kühl · wenig Text | [welt-glas-und-licht](2026-08-20-welt-glas-und-licht.md) |
| Fassung | dunkles Glas, ein Lichtleiter, kein reines Schwarz | [fassung-lichtleiter](2026-08-20-fassung-lichtleiter.md) |
| Größen | Main 760×430 · Active 700×420 · Kachel 260×84 | [groessen-alle-drei](2026-08-20-groessen-alle-drei.md) |
| Sprache | **App englisch**, Docs/Abnahmen/Commits deutsch | `CLAUDE.md` |

## Der nächste Schritt, konkret

`werkzeug/wireframe-main.html` ist offen und läuft auf der echten Größe
760×430. **Der User hat noch keine Disclosure-Mechanik gewählt.** Das ist die
nächste Frage — sonst nichts.

Gemessener Stand auf 760×430 (in allen drei Arbeitsschritten):

| Mechanik | Mix lesen | Befund | Urteilen |
|---|---|---|---|
| Schritt für Schritt | passt | passt | passt |
| Nur auf Abruf | passt | passt | passt |
| Focus + Context | −9 px | −20 px | passt |
| Semantischer Zoom | −46 px | passt | passt |
| Overview + Detail | −38 px | −30 px | −30 px |

Die drei Scheiternden scheitern **aus einem Grund**: sie halten alle 16
Quellen dauerhaft sichtbar (~190 px von 430). Es sind Beinahe-Treffer — mit
12 statt 16 dauerhaft sichtbaren Quellen würden sie passen. **Diese Zahl darf
Claude nicht stillschweigend kleiner drehen**; wieviele Quellen dauerhaft
sichtbar bleiben, ist eine Entwurfsentscheidung des Users.

## Was danach kommt

1. Wireframe **Active-Probe-Editor** (700×420), dann **Passive-Kachel** (260×84).
   Bei der Kachel ist jeder Pixel mit der Instanzenzahl multipliziert — zwölf
   Busse kosten 12,6 % des Schirms.
2. Stufe 2 Schritt 2 „Early Visual Layer", dann Schritt 3 „UI Design Stage".
3. Erst danach Licht und Material. **`werkzeug/licht.html` ist stillgelegt**
   und sein Modell ist falsch (vier sich ausschließende Herkünfte). Licht ist
   additiv, kommt auch aus UI-Elementen selbst, und Material entscheidet die
   Antwort darauf. Vor Gebrauch neu bauen —
   [korrektur-reihenfolge-und-lichtmodell](2026-08-20-korrektur-reihenfolge-und-lichtmodell.md).
4. `werkzeug/stilbefragung.html` zeigt noch die verworfene Welt und muss vor
   dem nächsten Einsatz umgestellt werden.

## Drei Fallen, in die ich in dieser Session getappt bin

Sie stehen hier, damit die nächste Session sie nicht wiederholt.

1. **Auf der falschen Ebene gefragt.** Erst Radien innerhalb einer Welt, die
   ich selbst gewählt hatte. Regel: erst Welt, dann Fassung, dann Werte.
2. **Stufen übersprungen.** Licht verhandelt, bevor die Anordnung stand.
   Der Arbeitsplan hat deshalb jetzt Phase 0.5, und `docs/DESIGN-GESETZE.md`
   ist Pflichtlektüre vor jeder Sichtbarkeitsentscheidung.
3. **Behauptet statt gemessen.** Mindestgrößen als Konstanten hingeschrieben;
   ein Kastendeckel lag unter der Inhaltshöhe und war durch keine
   Fenstergröße heilbar. Konsequenz im Code: elastische Dinge bekommen `1fr`,
   feste Textblöcke `auto`, und Werkzeuge **suchen** Mindestmaße per Messung,
   statt sie zu behaupten.

## Offen und ungefragt

- Wieviele Quellen dürfen dauerhaft sichtbar bleiben?
- Ist 760×430 die kleinste angebotene Stufe? (75 % = 570×322 schneidet ab.)
- Braucht die Passive-Kachel bei vielen Instanzen eine kleinere Stufe?
