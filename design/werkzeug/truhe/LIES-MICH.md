# Truhe — die Assettruhe, aufgeschlossen

`CLAUDE.md` Regel 1: `assets/` ist vor **jeder** Gestaltungsentscheidung die
erste Quelle. Gemessen am 2026-08-21 lag darin nichts, was eine Sitzung öffnen
kann — `110.sketch`, `110_colorAdj.psd`, `297 … .fig`, `301.fig`. Genau die
zwei Referenzen, die `werkzeug/weltenwahl.html` „direkte Übersetzungen" nennt,
konnte niemand nachsehen. Eine Regel, die man nicht befolgen kann, wird nicht
befolgt.

Hier liegen deshalb **abgeleitete Vorschauen**. Die Truhe selbst ist
unangetastet — Prüfsummen vor und nach dem Aufschließen identisch. Vorbild ist
`werkzeug/textur/korn.png`: herauskopiert, aufbereitet, Original in Ruhe
gelassen.

Neu erzeugen: `python werkzeug/truhe/aufschliessen.py` (braucht Pillow, sonst
nichts). Das Skript liegt hier, weil `PRUEFLISTE.md` den benannten Verlust
festhält — *„die Skripte waren sitzungslokal und sind weg"*.

---

## 301.zip — „Neumorphic Dark Kobs" *(Schreibweise des Autors)*

Voicu Apostol. Eigene Beschreibung: *„A minimalistic knob kit for **Audio
VST's** or any Neumorphic dark app."* Vektor, für Figma, Schrift **Outfit**.

**Das ist die Vorlage der Welt „Dunkles Gerät"** aus `weltenwahl.html` — und
sie ist keine allgemeine Dark-UI-Referenz, sondern **explizit für
Audio-Plugins gemacht**. Das ist der stärkste Beleg in der ganzen Truhe.

| Datei | was |
|---|---|
| `301-vorschau.png` 2340×1560 | Verkaufsbild, Titel wörtlich „NEUMORPHIC DARK KOBS · **13 VOLUME BUTTONS**": Knöpfe auf mattem Anthrazit, feines Korn |
| `301-thumbnail.png` 385×400 | Kontaktbogen der Kit-Kacheln, jede mit `OUTPUT` beschriftet |
| `301-bild-c20d72f0.png` 1600×1200 | **die Kornstruktur selbst** — Quelle von `werkzeug/textur/korn.png` |
| `301-bild-649cfbec.png` 400×300 | zweite Kornkachel, viel glatter |
| `301-readme.txt` | Beschreibung, Schrift, Autor |

**Gesehen, nicht angenommen:**
- Grund **fast schwarz**, laut `meta.json` des `.fig` exakt
  `rgb(25, 26, 30)` ≈ `#191A1E`.
- **Ein einziger Akzent, grün**, und immer als *Wert*: durchgehender Ring,
  gepunkteter Ring, einzelner Punkt. Nie als Dekoration.
- Der Zeiger ist ein **Haarstrich in Weiß**, kein farbiges Element.
- Tiefe kommt aus **weichem Doppelschatten** (hell oben links, dunkel unten
  rechts) — Neumorphismus, nicht Glas.
- Korn liegt über **allem**, auch über den Knöpfen.
- **Nachgemessen:** `301-bild-c20d72f0.png` hat Graumittelwert **128,00** bei
  Streuung 19,1 — `werkzeug/textur/korn.png` hat 127,95 bei 18,3. Damit ist
  belegt, was `werkzeug/LIES-MICH.md` bisher nur behauptete: **das ist die
  Quelle.** Die zweite Kachel (`649cfbec`, Streuung 6,2) ist es nicht.
- Keine Stückzahl aus dem Bild geraten: Der Kontaktbogen ließ sich nicht
  sauber in Kacheln zerlegen (Alphaerkennung fand 3×3 und schnitt die letzte
  Reihe ab). Die **13** oben ist deshalb zitiert, nicht gezählt.

## 110.zip — heller Rundregler, Thermostat

Kein readme. Zwei Fassungen derselben Sache: `110.sketch` (ist selbst ein ZIP,
darin `previews/preview.png`) und `110_colorAdj.psd` (farbkorrigiert).

**Das ist die Vorlage der Welt „Helles Objekt".**

| Datei | was |
|---|---|
| `110-vorschau.png` 2048×655 | zwei Zustände nebeneinander: Wetter · Heizen |
| `110-psd.png` 2340×1560 | dieselbe Scheibe groß, farbkorrigiert |
| `110-bild-97ba0f24.png` 2000×1300 | Hintergrundfläche |
| `110-bild-645dd31a.png` 916×383 · `110-bild-9e081595.png` 426×50 | Bauteile |

**Gesehen, nicht angenommen:**
- Grund **fast weiß mit weicher Vignette**, kein reines Weiß.
- **Ein warmer Akzent** als Verlauf orange → rot, und er ist der **Wert**
  (der zurückgelegte Bogen), nicht der Rahmen.
- Zwei Zahlen am Ring: `19` hell auf dunkel, `23` dunkel auf dunkel —
  Sollwert und Istwert, unterschieden durch Kontrast statt durch Farbe.
- Beschriftung `HEATING` **winzig, in Versalien, ohne Farbe**.
- Die Ziffer in der Mitte ist **hauchdünn und sehr groß** — die einzige
  Stelle, an der Typografie Charakter zeigt.
- Bedienung sind zwei **Haarstrich-Winkel** `‹ ›` links und rechts.

## 297.zip — „Campfire", Dark Social UI-Kit

Voicu Apostol. 23 Elemente und Widgets, Schrift **Roboto**, für Website oder
Mobil-App.

**Keine Vorschau vorhanden, und das bleibt so.** Die `.fig` ist im alten
`fig-kiwi`-Format; die Gestaltung liegt darin als Vektor und ist ohne Figma
nicht zu rendern. Was das Skript findet, sind nur die **eingebetteten
Bitmaps** — und die sind Dekoration, nicht das Kit:

| Datei | was |
|---|---|
| `297-bild1.png` 978×1434 | Neon-Tapete, blau/violett, gestreifte Wellenform |
| `297-bild2.png` 245×359 | derselbe Stoff, kleiner |

**Wer 297 wirklich beurteilen will, braucht Figma.** Bis dahin gilt: es ist
ein **Social**-Kit, nicht Audio — die schwächste Passung der drei, und der
naheliegendste Kandidat für eine **Antireferenz** im Designvertrag. Das ist
eine Beobachtung, keine Entscheidung; entschieden wird das mit dem User.

---

## Was hier NICHT steht

Kein Urteil darüber, welche Referenz gewinnt. Welt und Fassung sind
**Arbeitsannahmen** und bleiben es bis zum Prüfpunkt auf 760×430
(`abnahmen/LIES-MICH.md`). Diese Mappe macht sie nur **sichtbar** — damit die
nächste Runde an Bildern gemessen wird statt an Erinnerung.
