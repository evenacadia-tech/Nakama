# 2026-08-21 — Auftrag: Nakama Suna in Figma entwerfen, in der Sprache von Probeeq

> ## Abnahme (bindend) — neues User-Wort, das den Designvertrag für Suna ergänzt
>
> Der Designvertrag vom selben Tag sagt: *„Für Nakama Suna liegt noch kein
> Figma-Stand vor. Bis er kommt, gibt es für Suna nur die abgenommene Größe
> 260×84 und den Pflichtinhalt der Spezifikation. Nichts davon wird ohne Stand
> entworfen."* Dieses Wort öffnet genau diesen Punkt: der User beauftragt
> Claude, den Suna-Stand **in Figma** zu entwerfen — nicht frei, sondern in der
> Sprache des markierten Probeeq-Stands.

## Das wörtliche Urteil

> „ich habe das design des Nakama Probeeq markiert. deine aufgabe ist es in der
> selben visuellen Sprache die passive Sonde zu designen. Der Probeeq ist der
> neue Name und das neue design der aktiven Sonde. Die haupt Nakama app wird an
> anderer stelle genauso designt. ziel ist es die selbe detailtiefe zu erreichen
> und die designsprache zu treffen bei treue zum techniklayout.
> Arbeite direkt in figma"

Markiert war in der Figma-Datei `fable-dummy` (Seite „00 — Lumia Material Lab")
der Knoten **6013:4819** `Host / Ground` → `Active Probe Editor / 700x420 /
Unibody` — der Probeeq-Stand, der auch als `assets/figma/2026-08-21-probeeq.png`
in der Truhe liegt.

## Was damit festliegt

1. **Die Referenz ist Probeeq, nicht Gen.** Der Gen-Export vom 2026-08-21
   (`assets/figma/2026-08-21-gen.png`, cyan, Monospace-Versalien) ist die
   *ältere* Sprache; die Main-App „wird an anderer Stelle genauso designt" wie
   Probeeq. Für Suna gilt ausschließlich die Probeeq-Grammatik: Graphit-Unibody
   mit eingelassener Glas-Messfläche, Chin mit Link-Apertur, Status,
   Wortmarke; Geist + Geist Mono; Cyan nur als Leuchtsaum (nie als Fläche);
   Zustände hohl / gefüllt / gestrichelt.
2. **Claude entwirft Suna direkt in Figma**, in derselben Datei, neben dem
   Probeeq-Stand. Der Entwurf ist ein **Vorschlag von Claude**, kein
   abgenommener Stand — abgenommen ist er erst, wenn der User ihn (ggf. nach
   eigener Überarbeitung) exportiert und als `assets/figma/JJJJ-MM-TT-suna.png`
   in die Truhe legt. Bis dahin steht in `CLAUDE.md`: „Figma-Entwurf von
   Claude, nicht abgenommen".
3. **Maßstab: dieselbe Detailtiefe wie Probeeq** — nicht nur die Kachel im
   Schönfall, sondern Zustände, Grenzfall, Stapel mehrerer Instanzen, 4×-Zoom.
4. **Treue zum Techniklayout:** 260×84 bleibt (`2026-08-20-groessen-alle-drei.md`);
   Pflichtinhalt ist exakt `docs/oberflaechen-spezifikation.md` §3 — Name und
   Rolle des Busses, Messposition (Insert / PRE / POST), Verbindung + Frische,
   Hinweis bei Problemen (doppelte Instanz, kein Kontakt). Nichts darüber
   hinaus: kein Pegel, kein Spektrum, kein Bedienelement, das die Spezifikation
   nicht nennt.

## Ergebnis — was in Figma liegt (2026-08-21, Claude)

Figma-Datei `fable-dummy`, Seite „00 — Lumia Material Lab", neuer Rahmen
**`Lumia Material Lab / Study 03 — Suna`** (Node **6039:2**, 1600×1760) unterhalb
von Studie 01, im Dokumentformat der Studien des Users (Kopf mit Signalmarke,
Abschnittslabels, Zustände mit Name/Notiz, Entscheidungsliste, Entscheidungstor;
Blatt-Chrome deutsch, Produkttext englisch).

| Was | Node | Inhalt |
|---|---|---|
| `Host / Ground` 900×520 mit **`Passive Probe Tile / 260x84 / Unibody`** | 6040:4 → **6040:5** | der Schönfall: Bus `PIANO` · Rolle `CARRIES` · Position `POST` · `● CONNECTED` · `MEASURING` · kein Hinweis. Ground wie bei Probeeq, derselbe Maßstab 1:1 |
| `Seam / 4x Zoom` 524×340 + `Seam / Caption` | 6044:93 | die linke Hälfte der Kachel vierfach (Klon 1040×336): Hohlkehle, Korn, Apertur, Wortmarke |
| `States / Ground stale` | 6043:5 | verbunden, Messung veraltet: Apertur gefüllt **ohne** Saum, rechts `STALE · 40 s` |
| `States / Ground no contact` | 6043:34 | Apertur **hohl** (weiß 0,3 wie Probeeq „not linked"), `NO CONTACT`, `STALE · 2 min`, Hinweis `⚠ NOT DELIVERING TO GEN` |
| `States / Ground duplicate` | 6043:63 | verbunden und messend, Hinweis `⚠ DUPLICATE INSTANCE` |
| `States / Ground long name` | 6043:92 | Grenzfall: `BACKING VOX WIDE L+R` → `BACKING VOX…` (Kürzung nach 84 px), Rolle `SUPPORTS`, Position `PRE`, veraltet |
| `Instances / Ground 3 stacked` 340×348 | 6044:4 | drei Kacheln mit 8 px Fuge: Piano / Synth Pad (MERGED, veraltet) / Lead Vox (LEADS, INSERT) — der Scan über feste Spalten |
| `Delta / List` | 6045:4 | neun Zeilen: was 1:1 aus Probeeq kommt, was angepasst ist, was nicht übernommen wurde, was offen ist |

**Die Kachel, in Zahlen (1:1):** Unibody 260×84, Außenradius 14 · Glas
(`Glass / Measurement Plane`) 248×50 bei (6,6), Innenradius 8 — konzentrisch wie
Probeeq (10/10/20) · Kinn 28 px (Probeeq 40) · Display 56,8 % der Fläche
(Probeeq 85,6 %). Schale, Breitlicht, Korn (Truhe 301, gekachelt 0,15),
Innenkanten, Glasfüllung, Kantenglanz, Spiegelung, Vignette, Hohlkehle: **aus dem
Probeeq-Knoten geklont**, nicht nachgebaut. Einzige Anpassung an der Schale: der
Schwebeschatten ist halbiert (0/9 r20 Spread −4 statt 0/18 r40 Spread −8), weil
die Kachel 84 px hoch ist.

**Glasinhalt:** Kopfzeile `PASSIVE PROBE` (Geist Medium 8, Spationierung 1,2,
#808791 — exakt Probeeqs `ACTIVE PROBE · PIANO BUS`) · eine Parameterzeile in
Probeeqs Label/Wert-Grammatik (Geist Medium 7,5 / Geist Mono Medium 10):
`BUS` · `ROLE` · `POSITION`. Der Name hell (#E7E9EA), die Zustandswörter grau
(#8D939A, wie `DYN ON`). Feste Spalten BUS 84 · ROLE 56 · POSITION 43, Fuge 22.
Oben rechts der Hinweis (Glyphe + Geist Medium 7, warmes Weiß #E0D6D4), nur
bei Problem sichtbar — an der Stelle, an der Probeeq `PROTECTED` zeigt.

**Kinn:** Link-Apertur (5 px, #8D939A, Cyan-Saum r6 — Probeeqs Knoten geklont)
· `CONNECTED` (Geist Mono Medium 8, Spationierung 0,8, #80878F) · Wortmarke
`NAKAMA` / `SUNA` mittig (geklont aus dem **neuesten** Probeeq-Frame 6013:5925,
samt Shader-Effekt rot→blau, der im Truhen-Export sichtbar ist; der markierte
Knoten 6013:4819 trägt die flache Fassung) · rechts der Messstatus
`MEASURING` / `STALE · 40 s`.

**Mechanisch geprüft** (Prüfskript in Figma, 2026-08-21): Kachel exakt 260×84 ·
alle sieben Materialschichten vorhanden · alle Texte nur Geist / Geist Mono in
Probeeqs Größen (7 · 7,5 · 8 · 10 · 14) · kein Text ragt aus Glas oder Kinn, in
allen acht Kacheln, auch beim langen Namen · Pflichtinhalt §3 vollständig
verortet · Stapel 3 × 8 px · Zoom 1040×336.

**Was nicht entstanden ist, mit Absicht:** kein Pegel, kein Spektrum, kein
Bedienelement außer den drei Werten — die Spezifikation nennt für die Kachel
nichts davon. Keine Einsätze, keine `GLOBAL`-Taste. Kein Cyan als Fläche, kein
Signalrot.

## Was ausdrücklich offen bleibt (dem User vorzulegen — nicht still entschieden)

Jeder Punkt ist im Entwurf **eine** Lesart; die andere ist mit einem Handgriff
in Figma herstellbar. Die Reihenfolge ist die Reihenfolge der Tragweite.

1. **Aperturgrammatik: Probeeq statt Gen.** Der Entwurf folgt der Apertur des
   markierten Stands: gefüllt + Saum = verbunden und liefert · gefüllt ohne
   Saum = verbunden, Messung veraltet · hohl = kein Kontakt. Gens Kartengrammatik
   (`2026-08-20-zustaende-nur-ausnahme.md`) sagt für Quellenzeilen: hohl =
   veraltet, Strichlinie = getrennt. Beides nebeneinander heißt: „hohl" bedeutet
   in Gen etwas anderes als in Suna/Probeeq. Dieser Widerspruch steht schon im
   Designvertrag („Was ausdrücklich offen bleibt") und wird hier nicht
   aufgelöst — nur nicht vergrößert: Suna sagt dasselbe wie Probeeq.
2. **Lesart von „Messposition (Insert / PRE / POST einer Kette)".** Entworfen
   als **Dreistellung** `PRE · INSERT · POST` — wo in der Kette die Sonde sitzt
   (vor der Kette, als Einsatz mittendrin, am Ende). Die andere Lesart —
   „Insert" als FL-Mixerspur („Insert 4") plus PRE/POST — hätte einen
   Spurnamen gebraucht, den die Spezifikation nicht nennt. Entscheidung des
   Users; der Wert ist ein Wort, der Umbau kostet nichts.
3. **Rollenvokabular.** `LEADS · CARRIES · SUPPORTS · PROTECTED · MERGED` aus
   „führt / trägt / begleitet / geschützt / bewusst verschmolzen" (Spezifikation
   §1). Englische Wörter sind ein Vorschlag; die Spezifikation hat keine.
4. **Messstatus-Wörter.** `MEASURING` (frisch) · `STALE · 40 s` (veraltet,
   mit Alter) — die Alters-Abstufung (seit 10 s / seit 10 min) steht in der
   Zustände-Abnahme als offen; hier ist sie eine Zahl hinter dem Wort.
5. **Hinweis-Wortlaut.** `NOT DELIVERING TO GEN` (kein Kontakt) und
   `DUPLICATE INSTANCE` (doppelte Instanz). Kürzer ging nicht, ohne Sinn zu
   verlieren; länger passt nicht neben die Kopfzeile (140 px).
6. **Kopfzeile `PASSIVE PROBE`.** Familie (Probeeq: `ACTIVE PROBE · PIANO BUS`)
   — oder Redundanz zur Wortmarke, die bei zwölf gestapelten Kacheln zwölfmal
   dasselbe sagt? Sie trägt rechts den Hinweis; ohne sie bräuchte er einen
   anderen Ort.
7. **Feste Spalten statt Hug.** Probeeq hängt jede Spalte an ihren Inhalt; Suna
   hält BUS 84 · ROLE 56 · POSITION 43 fest, damit Rolle und Position über
   viele Kacheln untereinander stehen. Preis: eine Lücke hinter kurzen Namen.
8. **Schwebeschatten halbiert.** Einzige Abweichung an der Schale; Probeeqs
   Schatten in Originalgröße wäre so groß wie die Kachel selbst.
9. **Wortmarke mit Shader** aus dem neuesten Probeeq-Frame (6013:5925) statt
   der flachen aus dem markierten Knoten (6013:4819) — weil der Truhen-Export
   die Shader-Fassung zeigt. Die MCP-Vorschau rendert den Shader nicht; in
   Figma selbst ist er sichtbar.
10. **„Bestätigen" des Namens.** Die Spezifikation sagt „bestätigen/ändern";
    ein unbestätigter Zustand hat keine Darstellung bekommen — er ist nirgends
    beschrieben, und erfunden wird nichts.
11. **Kein Figma-Stand in der Truhe.** Der Entwurf ist ein Vorschlag in der
    Figma-Datei. Erst der Export des Users (`assets/figma/JJJJ-MM-TT-suna.png`)
    macht ihn zur Vorgabe; `assets/` wurde nicht angefasst.

## Sessionende 2026-08-21 (Suna-Runde)

Offen blieb: das Urteil des Users über den Entwurf in Figma — und die elf
Punkte oben. Kein Blatt in `prototyp/` für Suna; Phase 1b sagt Gen → Probeeq
→ Suna, und für Suna gilt der Stand erst mit dem Export.

## Bezug

- `abnahmen/2026-08-21-designvertrag.md` — Punkt „Für Nakama Suna liegt noch
  kein Figma-Stand vor", den dieses Wort öffnet
- `assets/figma/LIES-MICH.md` — der Probeeq-Stand, beschrieben
- `docs/oberflaechen-spezifikation.md` §3 — der Pflichtinhalt
- `abnahmen/2026-08-20-groessen-alle-drei.md` — 260×84
- `abnahmen/2026-08-20-zustaende-nur-ausnahme.md` — „nur die Ausnahme spricht"
