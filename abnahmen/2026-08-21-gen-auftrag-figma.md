# 2026-08-21 — Auftrag: Nakama Gen in Figma entwerfen, in der Sprache von Probeeq

> ## Abnahme (bindend) — User-Wort, das den Gen-Schritt der Phase 1b neu setzt
>
> Der Arbeitsplan sagte nach der Suna-Runde: *„Die nächste Session entwirft
> Nakama Gen in Figma, in der Probeeq-Sprache, von vorne."* Dieses Wort löst
> das ein: der User beauftragt Claude, den Gen-Stand **in Figma** zu entwerfen —
> nicht frei, sondern in der Sprache des markierten Probeeq-Knotens, so wie
> Suna in Studie 03. Der Gen-Export vom 2026-08-21 20:05
> (`assets/figma/2026-08-21-gen.png`, cyan) bleibt Verlauf und liefert die
> **Anordnung**, nicht den Look.

## Das wörtliche Urteil

Sessionende der Suna-Runde (2026-08-21):

> „okay dann closen wir hier und nächste session kommt Nakawa Gen. Codex hat
> das schonmal probiert, aber du kannst das besser deswegen machst du es von
> vorne"

Der Auftrag dieser Session, mit Link auf den markierten Probeeq-Knoten:

> „https://www.figma.com/design/ZbLODrj56o5OuoENwXbZe9/Testdummy?node-id=5015-5061
> create the nakama Gen design in the same design language than the probeeq ."

Markiert (und beim Lesen selektiert) war in der Figma-Datei **`Testdummy`**
(Key `ZbLODrj56o5OuoENwXbZe9`, Seite „00 — Lumia Material Lab", 0:1) der Knoten
**5015:5061** `Active Probe Editor / 700x420 / Unibody` in `5015:5060`
`Host / Ground`. Das ist **eine andere Datei** als `fable-dummy`
(Key `DvMbHg0MWCPwibDj8q6hI8`), in der Suna als Studie 03 liegt; `Testdummy`
enthält Studie 01 und drei Kopien von Studie 02, keine Studie 03. Der
Probeeq-Knoten ist bis auf die IDs derselbe wie der in der Truhe beschriebene
(Kopf `ACTIVE PROBE · PIANO BUS`, Draft-Kasten, acht Griffe, Bandzeile mit
zwölf Parametern, Fußzeile `CONNECTED · PAIRED`).

## Was damit festliegt

1. **Die Referenz ist Probeeq, gelesen aus dem Knoten, nicht aus dem Bild.**
   Alle Zahlen unten stammen aus `use_figma`-Lesungen des markierten Knotens
   (Füllungen, Effekte, Schriften, Spationierungen, Maße) — nicht geschätzt.
   (`searx` war lokal nicht erreichbar, HTTP 000; es gibt in dieser Datei keine
   Bibliotheksbehauptung, nur Messungen am Knoten.)
2. **Claude entwirft Gen direkt in Figma**, in derselben Datei, als eigene
   Studie. Der Entwurf ist ein **Vorschlag von Claude**, kein abgenommener
   Stand — abgenommen ist er erst, wenn der User ihn (ggf. überarbeitet)
   exportiert und als `assets/figma/JJJJ-MM-TT-gen.png` in die Truhe legt.
   Bis dahin steht in `CLAUDE.md`: „Figma-Entwurf von Claude, nicht abgenommen".
3. **Maßstab: dieselbe Detailtiefe wie Probeeq und Suna** — Schönfall,
   Arbeitsschritte, Zustände, Grenzfälle, 4×-Zoom, Delta-Liste.
4. **Treue zum Techniklayout:** 760×430 bleibt
   (`2026-08-20-groessen-alle-drei.md`); Pflichtinhalt ist exakt
   `docs/oberflaechen-spezifikation.md` §1; die bindenden Abnahmen vom
   2026-08-20 gelten (Overview + Detail · Karte trägt jede Quelle, gleich hoch ·
   Überschuss teilen, Mangel nicht · Vorhören markiert die Zeile · nur die
   Ausnahme spricht). Wo der alte Gen-Stand davon abwich (drei Zeichen, siehe
   Designvertrag), folgt der Entwurf der Abnahme und nennt das unten als offen.

## Ergebnis — was in Figma liegt (2026-08-21, Claude)

Figma-Datei `Testdummy`, Seite „00 — Lumia Material Lab", neuer Rahmen
**`Lumia Material Lab / Study 04 — Gen`** (Node **5025:1616**, 1600×6731) bei
(6000, 36) — rechts von allem Vorhandenen, im Dokumentformat der Studien des
Users (Signalmarke, Eyebrow, Titel, Status „STUDIE 04 / ENTWURF, NICHT
ABGENOMMEN", Abschnittslabels, Notizspalte je Rahmen, Delta, Entscheidungstor;
Blatt-Chrome deutsch, Produkttext englisch). **Studie 04**, weil Suna in der
Familie Studie 03 ist — auch wenn sie in der anderen Datei liegt.

| Was | Node | Inhalt |
|---|---|---|
| `Host / Ground` 900×520 mit **`Nakama Gen / 760x430 / Unibody`** | 5026:1618 → **5026:1619** | der Schönfall, Schritt **ADVISE**: fünf Quellen wie im Gen-Stand des Users (Lead Vox · Drums · Bass · Piano · Synth Pad veraltet), Befund 1 von 3, Fokusring auf Bass 120/250, `TEST READY`, `MEASURING 68 %`, `WIDTH UNAVAILABLE` |
| `Seam / 4x Zoom · Head` 700×340 | 5042:2358 | Körper-Ausschnitt (0,0 → 175,85) vierfach: Hohlkehle, Korn, Reiter mit Leuchtpunkt, `PROBE OVERVIEW`, erste Zellen |
| `Seam / 4x Zoom · Chin` 700×340 | 5042:2502 | Ausschnitt (265,345 → 440,430): Readout mit Leuchtpunkt, Wortmarke `NAKAMA / GEN` mit Shader, Naht Glas → Kinn |
| `States / Ground OBSERVE` | 5033:1618 → 5033:1619 | Schritt 1 „Mix lesen": Befundliste — 01 offen (Ort · `OBSERVED` · `BECAUSE` · `OTHER CAUSE` · `NEXT`, rechts `CERTAINTY HIGH`), 02 `UNCLEAR · MORE DATA NEEDED`, 03 `HIGH · NO CHANGE RECOMMENDED` |
| `States / Ground JUDGE (after apply)` | 5036:1616 → 5036:1617 | Schritt 3 „Urteilen" nach dem Anwenden: `BASS ●` (Marke bleibt), `UNDO · 1`, A/B level-gematcht (`HOLD FOR BEFORE`), Urteil `KEEP · DISCARD · UNCLEAR`, Detail der Quelle (Spektrum vorher/nachher, Lautheit, Dynamik, Spitze, Breite `—`, Verlauf) |
| `States / Ground AUDITION (held)` | 5037:1616 → 5037:1617 | Vorhören gehalten: Reiter `AUDITION ●`, Pille gedrückt, Zeile Bass markiert, Readout `AUDITIONING` |
| `States / Ground ALL FRESH` | 5041:1616 → 5041:1617 | die Abwesenheit spricht: `ALL 5 FRESH`, kein Hinweis, Synth Pad gefüllt |
| `States / Ground MIXED` | 5038:1616 → 5038:1617 | 7 Quellen, `BACKING VOX WIDE L+R` → `BACKING VOX…`, `2 STALE · ROOM MIC, BACKING VOX · 1 OFFLINE · SHAKER`, Zellen 16 / Fuge 6, Assistent 15-px-Raster, `6 OF 7 LINKED` |
| `States / Ground 16 SOURCES` | 5039:1616 → 5039:1617 | Belastungsprobe: 16 Zeilen mit Namen (Zellen 8 / Fuge 2 / Schrift 8), `8 STALE · 2 OFFLINE · BRASS, FX RETURN`, Assistent vollständig (13-px-Raster), `14 OF 16 LINKED` |
| `States / Ground ALL OFFLINE` | 5041:1760 → 5041:1761 | alles getrennt: alle Zeilen Strichlinie, Namen 50 %, Apertur hohl, `0 OF 5 LINKED · MEASUREMENT STALLED`, `5 OFFLINE`, kein Befund, kein Ring, keine Knöpfe, `OBSERVE ●` |
| `States / Ground NO SOURCE` | 5041:1912 → 5041:1913 | Leerzustand: `NO SOURCE CONNECTED` · `NOTHING TO MEASURE` (Wortlaut der Zustände-Abnahme), `0 SOURCES`, `0 LINKED`, Apertur hohl |
| Notizspalten (9), `Delta / List`, `Offen / List`, Entscheidungstor | 5043:1616 … 5046:1677 | je Rahmen Name und Begründung; Abgleich in fünf Gruppen; die 17 offenen Punkte unten, wörtlich auch im Blatt |

**Der Körper, in Zahlen (1:1):** Unibody 760×430, Außenradius 20 · Glas
(`Glass / Measurement Plane`) 740×380 bei (10,10), Innenradius 10 · Kinn 40 px
(wie Probeeq) · Glas 86,0 % der Fläche (740×380 / 760×430; Probeeq 85,6 %). Schale (#1B1D1F +
Verlauf), Breitlicht, Korn (Truhe 301, gekachelt 0,15, Soft Light 8 %),
Innenkanten, Glasfüllung (#0C0E10 + Verlauf), Kantenglanz, Spiegelung (Screen),
Vignette (Multiply), Schwebeschatten 0/18 r40 −8: **aus dem Probeeq-Knoten
geklont**, nicht nachgebaut, auf das Maß gestreckt. Der Schwebeschatten bleibt
in Originalgröße (Gen ist größer als Probeeq, nicht kleiner wie Suna).

**Glasinhalt (Innenrand 14 wie Probeeq):**

- **Ebenenzeile** (y 11): `OBSERVE · ADVISE ● · AUDITION · APPLY` — Geist
  Medium 8, Spationierung 1,2 (Probeeqs Tasten-Grammatik `GLOBAL`); inaktiv
  #858B90, aktiv #E7E9EA **mit Leuchtpunkt** (5 px #9A9195, Cyan-Saum r4 —
  Probeeqs `BAND 3 ●`). Oben rechts der degradierte Hinweis `⚠ WIDTH
  UNAVAILABLE` (Warnglyphe als Vektor + Geist Medium 7 / 0,9, warmes Weiß
  #E0D6D4 — der Suna-Ort, an dem Probeeq `PROTECTED` zeigt), nur wenn
  degradiert.
- **Karte** (y 30): `PROBE OVERVIEW` (Geist Medium 8 / 1,2, #858B90) links;
  rechts die Ausnahme (`1 STALE · SYNTH PAD`, Geist Mono Medium 8 / 0,8,
  #858B90) oder `ALL 5 FRESH`. Spalten `20 60 120 250 500 1k 2k 4k 8k` (Geist
  Mono Regular 8 / 0,2, #5C6269 — Probeeqs Skalen, Probeeqs Schreibweise).
  **Eine Zeile je Quelle, alle gleich hoch**: Nummer (Mono Regular 8,
  #5C6269) · Name (Mono Medium 10, #E7E9EA, feste Spalte 84, Kürzung `…`) ·
  Rolle (Mono Medium 10, #8D939A — Sunas Wortgrammatik) · neun Zellen 55×20,
  Radius 2, Fuge 4. **Zellen**: frisch = gefüllt (weiß 2,5 % + Wein #7E3D3E bei
  0–55 % nach Energie, sequenzielle Ein-Farb-Rampe über dem Glas, dunkel → hell
  wie `/dataviz` für Heatmaps vorschreibt) · veraltet = **hohl** (Kontur
  #E0D6D4 30 %) · getrennt = **Strichlinie** (22 %, 2/2, Name 50 % — Probeeqs
  Griff 7 „aus"). **Befundort** = Fokusring um die betroffenen Zellen (Kontur
  #9A9195 90 %, Radius 4, Cyan-Saum r8 45 % — Probeeqs gewählter Griff). Kein
  Zeilenmarker in Ruhe.
- **Arbeitsfläche** (unter einer Haarlinie weiß 5 %): `ADVISOR` ·
  `FINDING 1 OF 3` · rechts `CERTAINTY HIGH`; darunter die feste Fünferform als
  Label-neben-Wert (Label Geist Medium 7,5 / 0,9, #5C6269, Spalte 96; Wert
  Geist Mono 9 / 0,1 — `PRIORITY` und `SMALLEST TEST` Medium #E7E9EA, die
  übrigen Regular #B8BDC2) und als sechste Zeile `OTHER CAUSE` (Regular
  #858B90). Aktionen: Halten-Pille `HOLD TO AUDITION` (Fläche #9A9195 14 %,
  Kontur 40 %, r8, Geist Medium 7,5 / 1) · `APPLY` #B8BDC2 · `REJECT` #858B90 —
  Wort für Wort Probeeqs Draft-Kasten. Rechts der vorbereitete Zustand
  `● TEST READY · BASS −2.0 dB @ 220 Hz · Q 1.2` (Leuchtpunkt + Mono Regular 8).
- **Höhenbudget** (Schönfall, 5 Quellen): Ebenenzeile 11–21 · Karte 30–192
  (Zellen 20, Fuge 8) · Haarlinie 200 · Arbeitsfläche 208–355 (Zeilenraster 16)
  · Rest 25 (Probeeq: 24). Regel „Überschuss teilen, Mangel nicht": bei 7
  Quellen Zellen 16 / Fuge 6 und Raster 15, bei 16 Quellen Zellen 8 / Fuge 2 /
  Schrift 8 und Raster 13 — **der Assistent bleibt in allen drei Lastfällen
  vollständig** (6 Zeilen + Aktionen + Readout).

**Kinn:** Link-Apertur (5 px #9A9195, Cyan-Saum r6 — Probeeqs Knoten geklont;
hohl = weiß 30 % wenn nichts liefert) · `5 LINKED · MEASURING 68 %` (Geist Mono
Medium 8 / 0,8, #858B90 — Messung läuft / unvollständig lebt hier) · Wortmarke
`NAKAMA` / `GEN` mittig (Probeeqs Textknoten geklont: SemiBold 8 #3A4150 80 %
über ExtraLight 14 #9F9F9F, Spationierung 2,6, **samt Shader-Effekt**) · rechts
die Taste `SOURCES` (Geist Medium 8 / 1,2, #A8ADB2 — Probeeqs `GLOBAL`) als Tür
zu Quellenliste, Rollen, Suche, Pinning, Fokusgruppen · Einsatz `UNDO · 0`
(84×22, r11, Probeeqs Einsatz geklont; Beschriftung bei 0 auf 45 % gedimmt, bei
`UNDO · 1` voll) = „1 change applied" und Rückweg in einem Element.

**Mechanisch geprüft** (Prüfskript in Figma, read-only, 2026-08-21; Rückgabe
bytegleich als `figma-verify.json` im Sessionordner): alle **neun** Körper exakt
760×430, Glas [10, 10, 740, 380], Kinn 40 bei y 390 · alle sieben
Materialschichten in jedem Körper · **0** Schriftverstöße (nur Geist / Geist
Mono in 7 · 7,5 · 8 · 9 · 10 · 11 · 14) · **0** Texte außerhalb von Glas oder
Kinn — auch bei 16 Quellen und beim langen Namen · Zeilenhöhen je Karte gleich
(5 × 20 · 7 × 16 · 16 × 8). Die Prüfung wurde mit einem künstlichen Fehler
gegengeprüft (Inter 12 px, über den Glasrand gesetzt): beide Prüfungen schlagen
an, der Fehler wurde wieder entfernt. Screenshots: Studie gesamt (1600×6731),
Schönfall 1:1 und 2×, Probeeq-Original zum Vergleich — im Sessionordner.

**Was nicht entstanden ist, mit Absicht:** kein Kurvenfeld, keine Band-Griffe,
keine Bandzeile, kein Schutzbereich, kein `BYPASS` (Probeeq-Inhalt). Kein
Draft-Kasten — der Draft *entsteht* in Gen, `SMALLEST TEST` ist er. Kein Cyan
als Fläche oder Unterstrich, keine Monospace-Versalien-Kopfzeile, keine cyan
umrandeten Knöpfe, keine `FOCUS`-Zeile, kein `LIVE` je Zeile, kein
`5 / 5 LINKED` im Kartenkopf (→ Kinn), kein Zeilenmarker in Ruhe — der alte
Gen-Stand gibt die Anordnung, nicht den Look. Kein „Ask Claude", keine
KI-Erklärung (Designvertrag 9).

**Gemessen und damit erledigt:** „Karte mit Namen, 16 Quellen = 308 px" war
die einzige Zahl in `werkzeug/LIES-MICH.md` ohne Beleg. In Figma: 16 Zeilen
**mit** Namen brauchen **158 px** (Zellen 8, Fuge 2, Schrift 8). Die Namen
passen — zum Preis von 8-px-Schrift im 10-px-Raster, sichtbar im Blatt.

**Ein Vorfall, festgehalten:** beim ersten Lesen (Dump der Füllungen) trug die
Schale des markierten Knotens drei Füllungen — Solid, Verlauf **und eine
Shader-Füllung** (`cd43ef0a…/433`, Lichtstreifen). Der Klon übernahm sie und
zeigte im Rendering einen weißen Diagonalstreifen, das Original nicht. Beim
zweiten Lesen trug das Original **keine** Shader-Füllung mehr. Der Gen-Klon
folgt dem aktuellen Stand (Solid + Verlauf). Der Wortmarken-Shader
(`faabfff4…/416`, rot → blau) ist geklont und rendert nur in Figma selbst.

## Was ausdrücklich offen bleibt (dem User vorzulegen — nicht still entschieden)

Jeder Punkt ist im Entwurf **eine** Lesart; die andere ist mit einem Handgriff
in Figma herstellbar. Die Reihenfolge ist die Reihenfolge der Tragweite.

1. **Zellen-Grammatik gegen Apertur-Grammatik.** Zellen: hohl = veraltet,
   Strichlinie = getrennt (Abnahme Gen 20.08.). Apertur in Probeeq/Suna: hohl =
   kein Kontakt. Derselbe Widerspruch wie in Suna Punkt 1 — nicht aufgelöst,
   nicht vergrößert.
2. **Die vier Ebenen als Reiter oben links im Glas** — dort, wo Probeeq seine
   Identitätszeile (`ACTIVE PROBE · PIANO BUS`) hat. Gen trägt keine
   Identitätszeile; die Wortmarke im Kinn ist die Identität. Alternative: die
   Reiter als Tasten im Kinn rechts (Platz ist da, 220 px).
3. **Der Zeilenmarker ist dem Vorhören vorbehalten** (Abnahme 20.08.); der
   Befundort ist der Ring um die Zellen (Probeeqs Fokusring). Der alte Gen-Stand
   zeigte den Marker in Ruhe auf der Fokusquelle — das ist die dritte der im
   Designvertrag benannten Abweichungen, hier zugunsten der Abnahme gelesen.
4. **`1 STALE · SYNTH PAD`** — die Ausnahme nennt den Namen (Abnahme: Namen bis
   zwei je Sorte), obwohl die Zeile ihn inzwischen trägt. Redundanz oder
   Sicherheit? (Zweite Designvertrags-Abweichung, hier zugunsten der Abnahme.)
5. **Wortlaut der Aktionen:** `HOLD TO AUDITION / APPLY / REJECT` (Probeeq)
   statt `AUDITION / HOLD · APPLY · DISCARD` (alter Gen-Stand). Eine Familie,
   ein Wortschatz — oder Gens eigener?
6. **Sicherheit und Alternativursache haben jetzt einen Ort:** `CERTAINTY
   HIGH` im Befundkopf, `OTHER CAUSE` als sechste Zeile nach der festen
   Fünferform. Beide Pflichtangaben fehlten im alten Stand; der Ort ist neu.
7. **`SOURCES`-Taste** als Tür zu Quellenliste, Rollen, Suche, Pinning,
   Fokusgruppen — die Tür ist gezeichnet, der Raum dahinter nicht.
8. **`UNDO · n`** im Kinn = „1 change applied" + Rückweg in einem Element; bei 0
   gedimmt. Ein Verlauf dahinter ist nicht gezeichnet (die Spezifikation kennt
   ihn nur für Probeeq).
9. **Rollenwörter** `LEADS · CARRIES · SUPPORTS · PROTECTED · MERGED`
   (Suna-Vorschlag, Punkt 3 dort) als Wort je Zeile; `LIVE` nicht übernommen —
   frisch schweigt (Abnahme). Ohne zugewiesene Rolle stünde dort `—`.
10. **Wein als Energiefläche** der Zellen (Probeeqs Kurvenfüllung), Rampe
    0–55 % — ob die Stufen bei 1:1 auf dem Zielbildschirm lesbar genug sind,
    entscheidet der Blick des Users, nicht die Zahl.
11. **Drei Sorten Zustand, drei Orte:** degradierter Hinweis oben rechts im Glas
    (Suna-Ort, Warnglyphe) · Messung im Kinn links · Ausnahmen je Quelle im
    Kartenkopf rechts. Der alte Stand hatte alles in einer Kopfzeile.
12. **`FOCUS / BASS / 180–260 HZ`** aus dem Kartenkopf nicht übernommen: der Ort
    steht als Ring in der Karte und in der `PRIORITY`-Zeile. Spalten `1k 2k 4k
    8k` in Probeeqs Schreibweise (statt `1K 2K 4K 8K`).
13. **Befund-Navigation** (Befund 2 von 3 öffnen) nur über `OBSERVE` — kein
    `‹ ›` im Assistenten. Und die **Detailansicht** einer Quelle lebt im Schritt
    „Urteilen"; eine eigene Detailansicht außerhalb des Urteils ist nicht
    gezeichnet.
14. **16 Quellen:** Namen bei 8 px, Zeilenraster 10 px — trägt, aber an der
    Grenze. 308 px (Behauptung) → 158 px (gemessen).
15. **Shader-Füllung der Schale** (siehe Vorfall oben): der Gen-Klon trägt sie
    nicht, weil der markierte Knoten sie beim zweiten Lesen nicht mehr trug.
    Falls sie gewollt ist, ist sie ein Handgriff.
16. **Studie 04, nicht 03.** Suna ist Studie 03 in `fable-dummy`; `Testdummy` hat
    keine Studie 03. Ob die beiden Dateien eine werden sollen, ist Sache des
    Users.
17. **Kein Export in der Truhe.** Erst der Export des Users macht den Entwurf
    zum Stand (CLAUDE.md Regel 4) — bis dahin „Figma-Entwurf von Claude, nicht
    abgenommen". Die Übersetzung in `prototyp/` (Phase 1b, Schritt Gen) beginnt
    gegen den dann exportierten Stand.

## Sessionende 2026-08-21 (Gen-Runde)

Offen blieb: das Urteil des Users über den Entwurf in Figma — und die 17 Punkte
oben. Kein Export in der Truhe, kein Blatt in `prototyp/`. Der nächste Schritt:
der User sieht die Studie 04 in Figma (bei x 6000, rechts von Studie 01),
überarbeitet oder exportiert; dann Phase 1b, Schritt Gen, Punkt 1 („Lesen,
nicht raten") gegen den exportierten Stand.

**Nachtrag 2026-08-22, Schluss der Session („Closen wir hier"):** Der User hat
die Bilder gesehen (Galerie, neun 1:1-Renderings) und die flache Wortmarke
bemängelt; gemessen ist der Shader auf allen Gen-Wortmarken identisch mit dem
Probeeq-Knoten — nur die erreichbaren Renderer führen ihn nicht aus. Sein
eigener Export (`Wordmarkgen.png`, 22.08. 01:48, 190×68 = 4× der Tinte) zeigt
das echte Rendering und liegt im Sessionordner, **nicht** in der Truhe (Regel
1). Daraus kam das Erfolgskriterium in Worten:
`2026-08-22-erfolgskriterium-exakt-wie-figma.md` („Statisches wird gebacken,
Dynamisches gezeichnet"). **Offen blieb:** das Urteil über Studie 04 und die 17
Punkte oben; der Gen-Export in die Truhe; die Skalierungsstufen; der
Schalen-Shader. **Nächster Schritt unverändert:** Export des Users →
`assets/figma/JJJJ-MM-TT-gen.png` → Phase 1b, Schritt Gen, Punkt 1.

## Bezug

- `abnahmen/2026-08-21-suna-auftrag-figma.md` — das Vorgehen, das hier
  wiederholt wurde; Sessionende-Wort dort
- `abnahmen/2026-08-21-designvertrag.md` — die drei Abweichungen des alten
  Gen-Stands, hier zugunsten der Abnahmen gelesen (offene Punkte 3, 4)
- `abnahmen/2026-08-20-mechanik-main-overview-detail.md` ·
  `2026-08-20-karte-alle-quellen.md` · `2026-08-20-vorhoeren-markierte-zeile.md`
  · `2026-08-20-zustaende-nur-ausnahme.md` — die bindenden Abnahmen
- `assets/figma/LIES-MICH.md` — Probeeq- und Gen-Stand, beschrieben
- `docs/oberflaechen-spezifikation.md` §1 — der Pflichtinhalt
- `docs/DESIGN-GESETZE.md` — Aufgabenliste nach Häufigkeit als Prüfliste
