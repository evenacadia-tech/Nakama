# design/visuell — die visuelle Phase in Figma

**Stand: 02.09.2026 · Grundrahmen und 21 Zustandsrahmen, Vorschlag von
Claude, nicht abgenommen.**

Seit dem User-Entscheid vom 02.09.2026
([../abnahmen/2026-09-02-wechsel-visuelle-phase-figma.md](../abnahmen/2026-09-02-wechsel-visuelle-phase-figma.md))
entsteht das visuelle Design beider Apps in Figma, in der Materialsprache der
Entwürfe vom 20. bis 22.08.2026. Die Layout-Wahrheit bleibt die technische
Skizze ([../skizze/LIES-MICH.md](../skizze/LIES-MICH.md)); dieses Blatt sagt,
was in Figma liegt, wie die Übersetzung gelesen wird und was noch fehlt.
Bindend wird ein Figma-Stand erst mit datiertem User-Wort in `../abnahmen/`.

## Wo es liegt

- Figma-Datei **`fable-dummy`**, Key `DvMbHg0MWCPwibDj8q6hI8`, Seite
  **„01 — Nakama 950×538 · Skizze in Materialsprache"** (Node `6116:2`).
- Drei Bühnen (`Host / Ground`, #121315) mit je einem Unibody:

| Fläche | Ground | Unibody | Maß | Szenario der Skizze |
|---|---|---|---|---|
| Gen Fläche 1 (Arbeitsname Overview) | `6116:3` | `6116:6` | 950×538 | PROPOSAL |
| Gen Fläche 2 (Arbeitsname EQ) | `6116:8` | `6116:11` | 950×538 | PROPOSAL |
| Probeeq-Kachel | `6116:13` | `6116:16` | 600×92 | PROPOSAL |

- Referenz der Sprache auf Seite „00 — Lumia Material Lab" derselben Datei:
  Probeeq-Editor 700×420 (`6053:445`), Suna-Kachel 260×84 (`6048:209`),
  Wortmarke mit Shader rot→blau (`6091:1101`, geklont in alle drei Rahmen),
  Materialstudie 01 (`3:12`, „Five surfaces. One hierarchy").
- Die Figma-Datei `Nakama-Design` (Depot vom 22./24.08.2026) ist für diese
  Phase Verlauf; der User hat beim Wechsel `fable-dummy` benannt.

## Übersetzungsregeln (Skizze → Material)

1. **Geometrie 1:1 aus der Skizze**, gemessen am 02.09.2026 mit Playwright
   (Viewport 1500×900, Skalierung 1): Kopf 52, Sources-Spalte 180 mit
   Filterfeld 28 und Zeilen 42 im Raster 47, Evidenz 738×312, Befund 738×142,
   Zielleiste 38, Kontextzeile 22, Graph 928×370, untere Zeile 34 mit `GLOBAL`
   78×30, `UNDO`/`REDO` 58×30, Halten 128×30, `APPLY` 100×30, `REJECT` 62×30;
   Probeeq 600×92 mit zwei Zeilen à 36 (`EQ` 84×28, `BYPASS` 104×28, Mix 76).
   Eine bewusste Abweichung: Die Kopf-Umschalter `OVERVIEW` / `EQ` (96×51)
   stehen auf beiden Flächen zentriert bei x 377 und 477. Die Skizze
   verschiebt sie mit der Breite der Session-Zeile (355,8 gegenüber 313,9);
   ein Umschalter muss auf beiden Flächen an derselben Stelle liegen.
2. **Schale, Glas, Kinn.** Der Unibody ist die Schale des Probeeq-Editors
   (Polycarbonat #1b1d1f mit Streiflicht, Korn `korn.png` als Boolean-Subtrakt,
   vier Schattenlagen). Gen trägt oben ein 52 px hohes **Kopfband** aus Schale
   mit Wortmarke, Umschalter und Session-Wahrheit; die Arbeitsfläche liegt
   im **Glas** (#0c0e10, Kantenglanz, Spiegelung, Vignette, Innenkante 10).
   Fläche 2 hat zusätzlich die Zielleiste im Kopfband und die untere Zeile als
   **Kinn** mit Einsatz-Tasten. Probeeq: Zeile 1 Schalenband (Wortmarke, Bus,
   Apertur, `EQ`, `BYPASS`, Mix), Zeile 2 Glas (Messwahrheit, Modus).
3. **Tastenarten.** *Einsatz* (Metallpille, voll gerundet, Geist Mono Medium
   8) für `UNDO`, `REDO`, `COPY →`, `HOLD TO AUDITION`, `APPLY · 10 s`, `EQ`,
   `BYPASS`; *Schlüssel* (nur Schrift im festen Rechteck) für `GLOBAL`,
   `REJECT`, die Ziele und die Reiter; auf Glas die *Kartenpille* (Halten) und
   die *Weinpille* (`SEND DRAFT → EQ`). Gewählt heißt Weinton (#7e3d3e 28 %
   mit Kontur #9e4e50) für das Bearbeitungsziel und Grauring für eine
   Ansichtswahl (Quelle). Aktiver Reiter: helle Schrift und Cyansaum. Nicht
   verfügbar: Beschriftung auf 45 %. Maße ändern sich in keinem Zustand
   (Tasten-Gesetz 25.08.2026).
4. **Zustände über die Apertur.** Punkt gefüllt mit Cyansaum = verbunden und
   frisch; gefüllt ohne Saum = veraltet oder teilweise; hohl = kein Signal
   oder kein Kontakt. Daneben steht immer das Wort (`FRESH`, `STALE`,
   `PARTIAL`, `LINKED`); Wörter, die Aufmerksamkeit brauchen, in Warmweiß
   #e0d6d4 (`STALE`, `READY TO SEND`, `REMOTE PROPOSAL READY`).
5. **Graph.** Basis-Kurve wein (#9e4e50 mit Cyanglühen) über Weinfüllung
   (#621a28, EQ 35 %, Evidenz 20 %), Draft grau gestrichelt (#8d939a), Referenz
   stahlblau gepunktet (#7f96a6, neu: die alte Sprache kannte keine zweite
   Kurve), Analyzer als ruhige Weißfläche 5,5 %, Schutzzone schraffiert mit
   Schloss. Griffe 15 px mit Nummer; gewählt hell (#b0b5ba) mit Cyansaum;
   dynamisch mit äußerer Kontur; aus gestrichelt, Nummer 50 %, Strich.
6. **Typografie und Farben.** Geist Medium 7,5–8 (Labels, Schlüssel,
   Spationierung 0,9–1,2), Geist Mono Medium 8–10 (Werte, Namen, Einsätze),
   Geist Mono Regular 8 (Skalen, Nebenwerte). Text #e7e9ea / #b8bdc2 /
   #8d939a / #80878f / #6e757d / #5c6269. Cyan (#00bed2) nur als Saum, nie als
   Fläche. Die Wortmarke kommt als Klon aus der Datei: `NAKAMA` Geist SemiBold
   8 über `GEN` beziehungsweise `PROBEEQ` Geist Light 14, Shader rot→blau.

## Belege

`belege/` enthält Bildschirmfotos aus Figma vom 02.09.2026: die Bühne samt
Schatten, als zweifach skalierter Klon gerendert (der direkte 2×-Export der
vektorreichen Gen-Rahmen brach über die MCP-Verbindung ab):

| Datei | Inhalt |
|---|---|
| `2026-09-02-v01-gen-flaeche-1-overview@2x.png` | Gen Fläche 1: Kopfband mit Wortmarke, `OVERVIEW` aktiv, Session; Sources-Spalte mit Filter, 16 Quellen (8 sichtbar, Scrollanzeige), `BASS` gewählt; Evidenz mit Ziel- und Referenzkurve, Zone 180–260 Hz, Fußzeile; Befund mit drei Zeilen, `ALL FINDINGS`, `READY TO SEND`, Halten, `REJECT`, `SEND DRAFT → EQ` |
| `2026-09-02-v01-gen-flaeche-2-eq@2x.png` | Gen Fläche 2: Kopfband mit `EQ` aktiv, Zielleiste (`MASTER`, `KICK`, `VOCALS`, `BASS` gewählt, `PIANO`, Radhinweis, `COPY →`), Kontextzeile, Kurvenfeld mit Analyzer, Schutzzone, Basis/Draft/Referenz, sechs Griffen (B2 aus, B3 gewählt und dynamisch, B5 dynamisch), Skalen, Kurvenschlüssel; Kinn mit `GLOBAL ⌄`, `MIX 92 % · AUTO`, `UNDO`, `REDO`, `DRAFT BASS · B3`, Halten, `APPLY · 10 s`, `REJECT` |
| `2026-09-02-v01-probeeq-kachel@2x.png` | Probeeq 600×92: Wortmarke, `BASS BUS`, Apertur `LINKED`, `EQ · ON` mit Punkt, `BYPASS · OFF`, Mix 92 %; Glaszeile `REMOTE PROPOSAL READY · B3 · 240 Hz · −1.5 dB · Q 1.20 · MODE · POST` |

## Zustände und Panels (zweiter Schritt, 02.09.2026)

Jeder Zustand ist ein eigener Klon des Grundrahmens auf derselben Seite:
Zeile 2 (y 1000) Gen Fläche 1, Zeilen 3 und 4 (y 1750 und 2500) Gen Fläche 2,
Zeile 5 (y 3250) Probeeq. Die Geometrie jedes Zustands wurde in der Skizze
mit Playwright angesteuert, gemessen und gegen den Grundzustand verglichen;
gebaut wurde nur der Unterschied.

| Zustand | Ground | Beleg | Was er zeigt |
|---|---|---|---|
| Gen 1 · Befundliste | `6132:2` | `2026-09-02-v02-gen-flaeche-1-befundliste@2x.png` | `ALL FINDINGS` gedrückt, drei Befunde als Liste im selben Körper (`01 … SELECTED`, `02 … READY`, `03 … MORE DATA`) |
| Gen 1 · Quelle ohne Befund | `6132:182` | `2026-09-02-v02-gen-flaeche-1-ohne-befund@2x.png` | `KICK` gewählt, Referenz `MASTER`, `FRESH · 2.0 s · NO FINDING`, keine Zone im Graph, `NO FINDING`, Halten und Draft nicht verfügbar |
| Gen 1 · Filtertreffer | `6132:362` | `2026-09-02-v02-gen-flaeche-1-filter@2x.png` | Filter `gui` mit Leeren-Taste, Kopf `2 MATCH · 16`, nur `GUITAR L` und `GUITAR R` |
| Gen 1 · Draft offen | `6132:542` | `2026-09-02-v02-gen-flaeche-1-draft-offen@2x.png` | `DRAFT OPEN · EQ`, Handgriff `OPEN IN EQ →` |
| Gen 1 · Kandidat | `6132:722` | `2026-09-02-v02-gen-flaeche-1-kandidat@2x.png` | `EQ CANDIDATE · 8.3 s`, Halten nicht verfügbar, `CANDIDATE · OPEN IN EQ →`, `REVERT` |
| Gen 1 · Link-Verlust | `6132:902` | `2026-09-02-v02-gen-flaeche-1-link-verlust@2x.png` | hohle Apertur, `SESSION LINK LOST`, `STALE · LINK LOST`, `BLOCKED · LINK`, `SEND BLOCKED` |
| Gen 2 · Band-Panel Dynamic | `6132:1082` | `2026-09-02-v02-gen-flaeche-2-band-panel-dynamic@2x.png` | Karte 276×112 unter B3: `ON`, Typ, `ST`, `DYN · ON` (offen, wein), `DEL`, `×`; `DYNAMIC ON`, Range, Threshold, Attack, Hold, Release |
| Gen 2 · Band-Panel Grundansicht | `6132:1277` | `2026-09-02-v02-gen-flaeche-2-band-panel-grundansicht@2x.png` | Panel atomar an B4 gebunden (Notch, `R`, `DYN · OFF`), Felder Freq, Gain, Q; B4 gewählt, B3 behält die Dynamic-Kontur; Transaktion `BASS · B4` |
| Gen 2 · Typwahl | `6132:1472` | `2026-09-02-v02-gen-flaeche-2-band-panel-typwahl@2x.png` | 2×3 Filterkurven im selben Körper, Bell gewählt |
| Gen 2 · Kanalmodus | `6132:1667` | `2026-09-02-v02-gen-flaeche-2-band-panel-kanalmodus@2x.png` | `ST` `L` `R` / `M` `S` im selben Körper, `ST` gewählt |
| Gen 2 · Global offen | `6132:1862` | `2026-09-02-v02-gen-flaeche-2-global-offen@2x.png` | Karte über dem Graph: Input, Output mit `AUTO → −0.8 dB`, `AUTO · ON`, Width, Mono Bass, `BYPASS · OFF`, Mix, `A/B · A`, Preset `‹ USER 04 ›`, Revision `REV 128 · DRAFT OPEN`; `GLOBAL` im Kinn wein mit Pfeil nach oben |
| Gen 2 · Preset-Liste | `6132:2057` | `2026-09-02-v02-gen-flaeche-2-preset-liste@2x.png` | Filter, `SAVE` (nicht verfügbar bei mitgelieferten), `SAVE AS`, `×`; `FACTORY` und `USER`, `USER 04` gewählt |
| Gen 2 · Kopie-Zielwahl | `6132:2252` | `2026-09-02-v02-gen-flaeche-2-kopie-zielwahl@2x.png` | Zielleiste als `COPY BASS CURVE TO`, `PIANO` gedrückt, `REPLACES THE PIANO CURVE`, `CONFIRM COPY`, `CANCEL` |
| Gen 2 · Halten | `6132:2447` | `2026-09-02-v02-gen-flaeche-2-halten@2x.png` | Einsatz gedrückt `RELEASE TO RETURN` (versenkt, Cyansaum); der Draft trägt Saum und Breite, die Basis tritt zurück; Maße unverändert |
| Gen 2 · Kandidat | `6132:2642` | `2026-09-02-v02-gen-flaeche-2-kandidat@2x.png` | `CANDIDATE · 8.3 s` im Kopf, `CANDIDATE` in der Transaktion, `CONFIRM · 8.3 s`, `REVERT`, Halten nicht verfügbar |
| Gen 2 · Bestätigt | `6132:2837` | `2026-09-02-v02-gen-flaeche-2-bestaetigt@2x.png` | `CONFIRMED · REV 128`; keine Draft-Kurve, Kurvenschlüssel ohne `DRAFT`, Transaktionszeile verschwunden |
| Gen 2 · Link-Verlust | `6132:3032` | `2026-09-02-v02-gen-flaeche-2-link-verlust@2x.png` | `SESSION LINK LOST · LOCAL CONFIRMED · REV 128`, Kontext `LINK LOST`; `COPY →`, `GLOBAL`, `UNDO`, Halten, `APPLY`, `REJECT` nicht verfügbar; `BLOCKED` |
| Probeeq · EQ aus | `6132:3227` | `2026-09-02-v02-probeeq-eq-aus@2x.png` | `EQ · OFF` mit hohlem Punkt, Bypass und Mix nicht bedienbar, Glaszeile `EQ OFF · PASSIVE MEASUREMENT · SIGNAL · FRESH 1.2 s` |
| Probeeq · Link-Verlust | `6132:3260` | `2026-09-02-v02-probeeq-link-verlust@2x.png` | hohle Apertur `LINK LOST`, `LOCAL CONFIRMED · REV 128 · REMOTE LOCKED · EQ, BYPASS, MIX STAY LOCAL`; EQ, Bypass, Mix bedienbar |
| Probeeq · Kandidat | `6132:3293` | `2026-09-02-v02-probeeq-kandidat@2x.png` | `REMOTE CANDIDATE · 8.2 s` mit der laufenden Änderung |
| Probeeq · Bestätigt | `6132:3326` | `2026-09-02-v02-probeeq-bestaetigt@2x.png` | `CONFIRMED · REV 128` mit der bestätigten Änderung |

Panels liegen als Karte über dem Glas (#131619 bei 97 %, Kontur weiß 8 %,
Radius 7, Schatten), wie die Draft-Karte des alten Editors; Tasten darauf
sind Kartenpillen mit Radius 6, Werte sind dunkle Einlässe (#0c0e10), und
Spiegelung und Vignette des Glases bleiben über der Karte. Nicht verfügbar
heißt überall Beschriftung und Fläche auf 45 %.

Bewusste Abweichungen von der Skizze in diesem Schritt (Dichteprüfung des
Wortlauts, keine neuen Entscheide):

- `ALL FINDINGS` bleibt bei geöffneter Liste gedrückt statt in
  `CURRENT FINDING` umbenannt zu werden; die Taste behält 84 px
  (Tasten-Gesetz), die Skizze verbreitert sie.
- Ohne Befund zeigt der Evidenzgraph keine Zone, keinen Marker und keinen
  Punkt; die Skizze lässt sie stehen.
- Probeeq bei Link-Verlust sagt `LINK LOST` statt `BROKER OFFLINE`: dasselbe
  Wort wie Gen, und `BROKER OFFLINE` passt nicht zwischen Apertur und die
  festen Einsätze, ohne dass die Skizze diese verschiebt und verkleinert.
- Beim Halten wandern Saum und Breite von der Basis auf den Draft, weil der
  Draft dann hörbar ist; die Skizze verbreitert nur die Draft-Kurve.
- Positionen der Einsätze und Schlüssel bleiben in jedem Zustand gleich; die
  Skizze lässt sie mit der Textbreite wandern (Session-Zeile, Probeeq-Zeile 1).

Hinweis: Der Shader der Wortmarke (rot→blau) rendert nur in Figma selbst; in
Exporten und MCP-Vorschauen erscheint die Wortmarke flach. Das ist seit dem
22.08.2026 bekannt (`../abnahmen/2026-08-21-gen-auftrag-figma.md`, Nachtrag).

## Was noch nicht liegt

- **Flüchtige Zustände** der Skizze, die noch keinen Rahmen haben: `SAVE`
  mit `OVERWRITE USER 04 WITH THE CURRENT STATE?`, `SAVE AS` mit Namensfeld,
  Validierungszeile am Feld (`WIDTH · NOT A NUMBER, KEPT 100 %`), Graph-Hinweise
  (`BAND LIMIT · 8 OCCUPIED`, `CANDIDATE ACTIVE …`, `COPIED … TO …`) und
  Hover-Zustände; sie folgen bei Bedarf demselben Weg.
- **Silber-Schale** (neuere Fassung in der Datei): nur auf Zuruf.
- **Produktnamen** der beiden Gen-Flächen (Karte U23): `OVERVIEW` und `EQ`
  sind Arbeitsnamen.
- **Skalierungsstufen** 100/125/150/200 %: vergrößern dieselbe Geometrie;
  gebackene Assets je Stufe entstehen erst nach einer Abnahme.
- **Abnahme:** keine. Änderungen des Users im Figma-Stand sind neues
  User-Wort und werden hier datiert nachgetragen.
