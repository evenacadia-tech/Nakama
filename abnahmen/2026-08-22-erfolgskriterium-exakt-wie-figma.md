# 2026-08-22 — Erfolgskriterium: die Apps in FL Studio sehen exakt aus wie in Figma

> ## Abnahme (bindend) — das Erfolgskriterium in Worten
>
> Der Designvertrag vom 2026-08-21 führte „Erfolgskriterien in Worten“ als
> offen: der User hatte Bilder geliefert, keine Sätze. Dieser Satz schließt
> die Lücke. Er ist der Maßstab für jede Übersetzung in diesem Repo und für
> das Übergabepaket (Phase 5).

## Das wörtliche Urteil

Anlass: die Bildergalerie zur Gen-Studie zeigte die Wortmarke flach, weil
keiner der erreichbaren Renderer Figmas Shader-Effekt ausführt; der User hat
daraufhin seinen eigenen Export geschickt (`Wordmarkgen.png`, 22.08., 01:48 —
NAKAMA rot → blau je Buchstabe, GEN als dünne Kontur mit Verlauf) und gesagt:

> „mir ist es eigentlich vollkommen egal wie es im artifakt aussieht ,
> Hauptsache die Apps selbst in fl Studio sehen dann exakt aus wie in figma“

## Was damit festliegt

1. **Das Erfolgskriterium des Designs:** Nakama Gen, Probeeq und Suna sehen
   **in FL Studio exakt so aus wie in Figma** — Material, Schrift, Effekte,
   Wortmarke. Zwischenbilder (Galerien, Server-Renderings, Artefakte) sind
   dem User gleichgültig; sie beweisen nichts und dürfen nichts behaupten.
2. **Der Beweis ist der Pixelvergleich gegen den Figma-Export des Users**, bei
   1:1 auf dem Zielbildschirm (1920×1080, 100 %) und auf jeder Skalierungsstufe.
   Das gilt für das Blatt in `prototyp/` genauso wie für das Plugin: was der
   Prototyp anders zeigt als der Export, ist ein Fehler des Prototyps.
3. **Konsequenz für die Übersetzung — Statisches wird gebacken, Dynamisches
   wird gezeichnet.** Alles, was sich im Betrieb nie ändert, wird nicht
   nachgebaut, sondern **aus Figma exportiert und als Bild eingebettet**:
   - die **Wortmarken** (Shader-Effekt: Verlauf je Buchstabe, Kontur) —
     je Stufe ein Export, exakt die Pixel, die Figma rendert;
   - die **Schale samt Glas und Kinn** (Verläufe, Korn mit Soft Light,
     Innenkanten, Kantenglanz, Spiegelung mit Screen, Vignette mit Multiply,
     Schwebeschatten) — ein Körperbild je Stufe; Mischmodi und Korn lassen sich
     live nicht verlässlich gleich rendern, gebacken sind sie trivial;
   - **Leuchtsäume** (Cyan-Saum an Apertur, Leuchtpunkt, Fokusring) als
     kleine Sprites, wenn der Live-Schatten der Plattform sichtbar abweicht.

   Live gezeichnet wird nur, was Daten trägt: Texte, Zellen der Karte,
   Kurven, Griffe, Zustände. Diese Teile bestehen aus Flächen, Konturen,
   Strichlinien und Deckkraft — plattformunabhängig exakt reproduzierbar.
4. **Schrift:** Geist und Geist Mono werden in Prototyp und Plugin
   **eingebettet**, in genau Figmas Schnitten und Größen (7 · 7,5 · 8 · 9 ·
   10 · 11 · 14 bei 1:1) und Spationierungen. Keine Ersatzschrift, kein
   System-Fallback.
5. **Skalierung:** je angebotener Stufe (Empfehlung aus
   `2026-08-20-groesse-main.md`: 100 / 125 / 150 / 200 %) ein eigener
   Export-Satz der gebackenen Teile aus Figma — nicht hochgerechnet.
6. **Der Prototyp benutzt dieselben gebackenen Assets wie das Plugin.** So
   beweist er die Kette Figma → Asset → Darstellung, statt den Shader in CSS
   nachzuahmen. Assets liegen, sobald der User sie exportiert, datiert in der
   Truhe (`assets/figma/…`, Regel 1: die Truhe wird nie ungefragt verändert —
   der Wortmarken-Export vom 22.08. liegt deshalb noch im Sessionordner, nicht
   in `assets/`).

## Die eine Grenze, ehrlich benannt

**Text bei 7–10 px rendert keine zwei Engines pixelidentisch.** Figma, ein
Browser und die Plugin-Grafik setzen Hinting und Kantenglättung verschieden;
bei Geist Medium 7,5 px sind das sichtbare Grauwerte an den Kanten, nicht
Form oder Maß. Gebackene Wortmarken sind davon ausgenommen (sie sind Bilder).
Für den Rest gilt: Größen, Spationierung, Positionen und Farben exakt, und die
Abweichung wird am 1:1-Vergleich **gemessen und benannt**, nicht wegerklärt.
Falls der User auch hier null Abweichung will, ist der Weg derselbe: die
betroffenen Beschriftungen backen — dann sind sie Bilder, und Bilder sind exakt.

## Was ausdrücklich offen bleibt

- Die **Liste der Skalierungsstufen** (Empfehlung 100/125/150/200 %, keine
  Abnahme) und ob 75 % entfällt.
- Ob die Schalen-Shader-Füllung (Lichtstreifen, beim zweiten Lesen nicht mehr
  am Probeeq-Knoten) zum Design gehört — wenn ja, wird sie mitgebacken.
- Wo die Exporte erzeugt werden (vom User aus Figma, Export-Einstellungen je
  Stufe am Knoten) und wie sie benannt werden — Vorschlag folgt im
  Übergabepaket, Phase 5.
- Die technische Seite (wie das Plugin Bilder und Schriften einbettet) ist
  Sache des Technik-Repos; hier steht nur, **was** geliefert wird und **woran**
  es gemessen wird.

## Bezug

- `abnahmen/2026-08-21-designvertrag.md` — „Erfolgskriterien in Worten“ war
  dort offen; mit dieser Datei geschlossen
- `abnahmen/2026-08-21-gen-auftrag-figma.md` — die Wortmarke mit Shader, Beleg
  am Knoten (offener Punkt 15 dort: Schalen-Shader)
- `abnahmen/2026-08-20-groesse-main.md` — die empfohlenen Skalierungsstufen
- `docs/arbeitsplan.md` — Phase 1b (Prototyp) und Phase 5 (Übergabepaket)
- `werkzeug/PRUEFLISTE.md` — wie am Blatt gemessen wird

*Prüfstand 2026-08-22:* `searx` lokal nicht erreichbar (HTTP 000). Die Aussagen
zu Export-Stufen in Figma, zu Mischmodi in Plugin-Grafik und zur
Einbettbarkeit von Geist (SIL Open Font License) sind Stand meines Wissens,
nicht frisch gegengeprüft — gekennzeichnet, damit Phase 5 sie prüft.
