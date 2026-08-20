# 2026-08-20 — Sessionende: Stand und nächster Schritt

> **⚠ ÜBERHOLT.** Der aktuelle Stand steht in
> [2026-08-21-sessionende](2026-08-21-sessionende.md). Diese Datei ist der
> Stand von davor und wird nur noch als Verlauf gelesen — ihre Messtabelle
> war nachweislich in 6 von 15 Feldern falsch.

Diese Datei ist der einzige Wissenstransfer. Was hier nicht steht, ist weg.

## Wo wir stehen

**Phase 0 abgeschlossen** (Welt und Fassung), **Phase 0.5 abgeschlossen**
(Formfaktor und Größe). **Phase 1 begonnen**, aber nicht entschieden.

| Was | Entscheidung | Datei |
|---|---|---|
| Methode | visuell befragen, auf Weltebene beginnen | [methode-visuelle-befragung](2026-08-20-methode-visuelle-befragung.md) |
| Welt | **⚠ Arbeitsannahme** — Glas und Licht · Place · kühl · wenig Text | [welt-glas-und-licht](2026-08-20-welt-glas-und-licht.md) |
| Fassung | **⚠ Arbeitsannahme** — dunkles Glas, ein Lichtleiter | [fassung-lichtleiter](2026-08-20-fassung-lichtleiter.md) |
| Größen | Main 760×430 · Active 700×420 · Kachel 260×84 | [groessen-alle-drei](2026-08-20-groessen-alle-drei.md) |
| Sprache | **App englisch**, Docs/Abnahmen/Commits deutsch | `CLAUDE.md` |

## Wichtig: die visuelle Richtung ist NICHT entschieden

> „nein glas und licht ist nicht entschieden, es war zu dem zeitpunkt logisch.
> wir werden sehen wenn das layout steht ob es dann noch passt."

Welt und Fassung sind **Arbeitsannahmen mit Prüfpunkt**, keine Abnahmen. Sie
wurden an einem 448×310-Ausschnitt gewählt, als es weder Layout noch echte
Fenstergröße gab. Erneut vorgelegt werden sie, **sobald das Layout von Main
steht — auf 760×430, am echten Layout.** Sie dürfen dort fallen.

Claude darf sie bis dahin nicht als entschieden zitieren. Die Unterscheidung
Abnahme / Arbeitsannahme steht jetzt in [LIES-MICH.md](LIES-MICH.md).

## Der nächste Schritt, konkret

> **✔ ERLEDIGT 2026-08-20:** die Mechanik ist gewählt — **Overview + Detail**.
> Siehe [mechanik-main-overview-detail](2026-08-20-mechanik-main-overview-detail.md).
> Was unten steht, ist der Stand *vor* dieser Wahl.

`werkzeug/wireframe-main.html` ist offen und läuft auf der echten Größe
760×430. **Der User hat noch keine Disclosure-Mechanik gewählt.** Das ist die
nächste Frage — sonst nichts.

> **⚠ NACHGEMESSEN 2026-08-20, die Tabelle unten ist überholt.**
> Sie stand in 6 von 15 Feldern falsch, und ihre Diagnose war falsch. Zwei
> Gründe: das Werkzeug maß nur vier CSS-Klassen und übersah verschachteltes
> `overflow:hidden` (gemeldet 36 px, tatsächlich 111), und mehrere Kästen
> hatten einen Deckel unter der eigenen Inhaltshöhe. Beides ist behoben.
> Gemessene Grenzen waren: Focus+Context 11 (nicht 12), semantischer Zoom 13,
> Overview+Detail **nie** — dessen Rest hing an der Befundkarte, nicht an der
> Quellenzahl. Der Satz „mit 12 statt 16 würden sie passen" trifft auf keine
> der drei zu. Der aktuelle Stand steht in `werkzeug/LIES-MICH.md`; dort
> passen nach der Reparatur alle fünf, und sie unterscheiden sich nur noch
> darin, wieviel vom Mix **dauerhaft** stehen bleibt.

Gemessener Stand auf 760×430 (in allen drei Arbeitsschritten) — **überholt**:

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

0. **Vorläufiges als endgültig verbucht.** „Logisch zu diesem Zeitpunkt" ist
   keine Abnahme. Wo an einem Stellvertreter geurteilt wurde, gehört ein
   Prüfpunkt dazu — sonst wird eine Annahme zur Fessel.
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
