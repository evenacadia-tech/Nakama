# 2026-08-20 — Größe von Main: 760×430, skalierbar

## Das wörtliche Urteil

> „es müssen die beiden kleinsten sein, oder eine freie skalierungsform eben.
> so oder so die zweitkleinste ist das vernünftigste"

Die beiden kleinsten Kandidaten waren **Klein 520×300** und
**Kompakt 760×430**. Die zweitkleinste ist damit **760×430**.

## Was damit festliegt

1. **Grundgröße von Main: 760×430** (Seitenverhältnis 1,77 : 1).
   Auf dem Zielbildschirm 1920×1080 sind das **15,8 %** der Fläche.
2. **Skalierbar, nicht starr.** „Freie Skalierungsform" ist ausdrücklich
   zugelassen. Umsetzungsempfehlung (mechanisch, daher meine Entscheidung):
   **eine Grundgröße mit festen Faktoren** — 75 / 100 / 125 / 150 / 200 % —
   statt frei ziehbarem Rand. Grund: frei ziehbare GUIs sind in FL Studio
   unzuverlässig (Größe wird nicht gemerkt, springt nach dem Öffnen der
   Wrapper-Einstellungen zurück). Feste Faktoren wirken für den Nutzer wie
   freie Skalierung und sind zugleich verlässlich. So macht es auch FabFilter.
3. **Folge daraus, und das ist die wichtigste Zeile:**
   Bei 760×430 passt jede der sechs Aufgaben — „Alles gleichzeitig" passt
   **nicht**. Mit dieser Größe ist Progressive Disclosure keine Stilfrage
   mehr, sondern Bedingung.

## Die Messung

`werkzeug/formfaktor.html` rendert jede Aufgabe in einer gegebenen Größe und
misst auf jeder Ebene, ob Inhalt abgeschnitten wird. Kein Mindestmaß ist
behauptet; alle sind gesucht.

| Aufgabe | Häufigkeit | kleinste Größe ohne Abschnitt |
|---|---|---|
| 1 · Mix lesen | sehr häufig | 300×200 |
| 2 · Befund verstehen | Kernakt | **300×348** ← bindend |
| 3 · Versuch machen | folgt jedem Befund | 300×292 |
| 4 · Urteilen | folgt jedem Versuch | 300×200 |
| 5 · Quelle finden | gelegentlich | 300×200 |
| 6 · Rollen setzen | selten | 300×200 |
| ✕ Alles gleichzeitig | Vergleichsmaß | **870×410** |

**Grenze der Messung, ausdrücklich:** geprüft wird geometrischer Überlauf,
nicht Lesbarkeit. Die 300 px Breite sind die untere Suchgrenze — bei 300 px
wären neun Frequenzbänder mit Namen zwar geometrisch drin, aber nicht mehr
lesbar. 760×430 ist deshalb nicht „gerade so genug", sondern das kleinste
Maß **mit Luft**.

## Was ausdrücklich offen bleibt

- **Größe des Active-Probe-Editors** (Platzhalter 700×420) und der
  **Passive-Probe-Kachel** (Platzhalter 260×84). Die Kachel erscheint
  vielfach gleichzeitig — dort ist jeder Pixel teurer als bei Main.
- Ob 760×430 auch die *kleinste angebotene Stufe* ist oder ob es darunter
  noch eine gibt (75 % wären 570×322 — dort fehlen der Aufgabe „Befund
  verstehen" bereits Pixel, das wäre also keine gültige Stufe).
- Alles Gestalterische. Größe ist entschieden, Anordnung nicht.

## Bezug

- [2026-08-20-korrektur-formfaktor-zuerst.md](2026-08-20-korrektur-formfaktor-zuerst.md)
- `docs/DESIGN-GESETZE.md` — warum die Größe die Disclosure erzwingt
