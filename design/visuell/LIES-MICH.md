# design/visuell — die visuelle Phase in Figma

**Stand: 02.09.2026 (Abend) · Grundrahmen und 21 Zustandsrahmen nach der
Poliersession, Vorschlag von Claude, nicht abgenommen.**

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
   Seit der Poliersession (02.09.2026, Abend) liegt alle Geometrie auf ganzen
   Pixeln (Quellzeilen 42 im Raster 47 statt 42,4/47,4), Schalen- und
   Glasinhalt teilen eine Kantenspalte bei x 22 und 928 (Glas-Innenabstand
   12), und der Aktionsblock von Fläche 1 besteht aus drei Pillen à 30
   (Halten 107, `REJECT` 107, `SEND DRAFT → EQ` 220) statt der gestreckten
   Skizzenzellen 107,5×45,5. Die Kartenpillen der Band-Panels folgen ihrem
   Griff.
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
5. **Graph.** Seit der Poliersession sind beide Graphen gerechnet, nicht
   gezeichnet. Die Frequenzachse ist logarithmisch von 20 Hz bis 20 kHz
   (Fläche 2: x = 57 + log10(f/20) · 273,33 im Kurvenfeld, Fläche 1:
   x = 14 + log10(f/20) · 230,67 im Evidenzfeld), Dekadenlinien 100/1k/10k
   weiß 7 %, Zwischenlinien 2,5 %, Ränder 5 %, Ticks 20 · 50 · 100 · 200 ·
   500 · 1k · 2k · 5k · 10k · 20k. Die EQ-Skala auf Fläche 2 läuft von
   +12 bis −12 dB (12 px je dB, 0 dB bei y 182, Linie 10 %), die
   Analyzer-Skala links von 0 bis −72 dBFS (4 px je dB); die Einheit steht
   im obersten Wert (`+12 dB`, `0 dBFS`), alle Skalenwerte rechtsbündig.
   Kurven kommen aus Bandparametern (RBJ-Biquads bei 48 kHz): BASE B1 Bell
   45 Hz +1,5 dB Q 1,0 · B2 120 Hz aus · B3 Bell 240 Hz +1,0 dB Q 1,2 (Draft
   −1,5 dB) · B4 Bell 880 Hz +0,6 dB Q 0,9 (die Panelwerte der Skizze) · B5
   Bell 2,8 kHz −2,5 dB Q 1,0
   dynamisch · B6 High-Shelf 8 kHz +1,5 dB; Referenz PIANO Low-Shelf 90 Hz
   −4 dB, Bell 2,5 kHz +2 dB, High-Shelf 10 kHz +1 dB. Der Analyzer ist ein
   Bassbus-Spektrum (Grundtonbereich 45–110 Hz bei −15 dBFS, Obertonbuckel,
   Abfall zu −69 dBFS), die Schutzzone 3–6 kHz liegt auf ihren echten
   Frequenzen. Basis-Kurve wein (#9e4e50, 1,5 px, Cyanglühen), Draft grau
   gestrichelt, Referenz stahlblau gepunktet; die Weinfüllung ist ein
   senkrechter Verlauf von der Kurve bis zum Rasterboden (#621a28, Fläche 2
   42 → 10 %, Fläche 1 30 → 6 %). Griffe 15 px sitzen auf Frequenz und Gain;
   gewählt hell (#b0b5ba) mit Cyansaum, dynamisch mit äußerer Kontur 25 px,
   aus als gestrichelter Ring mit Nummer 50 % (kein Strich mehr). Fläche 1
   zeigt LTAS-artige Kurven (relative Skala 0 bis −60 dB über 180 px) für
   BASS, PIANO, KICK und MASTER; die Befundzone 180–260 Hz hat ihre echte
   Breite (37 px), Füllung 12 %, zwei Haarlinien, Marker und 11-px-Punkt bei
   240 Hz auf der Zielkurve, Beschriftung darüber; der Kurvenschlüssel steht
   oben links im Feld, auf beiden Flächen gleich gebaut.
6. **Typografie und Farben.** Geist Medium 7,5–8 (Labels, Schlüssel,
   Spationierung 0,9–1,2), Geist Mono Medium 8–10 (Werte, Namen, Einsätze),
   Geist Mono Regular 8 (Skalen, Nebenwerte). Seit der Poliersession sind
   die Grautöne auf feste Rollen gestrafft: #e7e9ea Primärtext, #b8bdc2
   Sekundärtext, #9ea8b2 Schlüsselschrift, #8d939a Status, #80878f Köpfe,
   Schalenlabels und stille Schlüssel (`REJECT`, inaktiver Reiter), #6e757d
   Labels und Skalen auf Glas, #5c6269 nur Platzhalter; Einsatz-Beschriftung
   #9a9296 in Ruhe und #e7e9ea aktiv oder primär, die Kartenpille auf Glas
   #b8bdc2. Einsätze tragen überall
   Geist Mono, Schlüssel überall Geist. Cyan (#00bed2) nur als Saum, nie als
   Fläche. Die Wortmarke kommt als Klon aus der Datei: `NAKAMA` Geist SemiBold
   8 über `GEN` beziehungsweise `PROBEEQ` Geist Light 14, Shader rot→blau.

## Belege

`belege/` enthält Bildschirmfotos aus Figma vom 02.09.2026: die Bühne samt
Schatten, als zweifach skalierter Klon gerendert (der direkte 2×-Export der
vektorreichen Gen-Rahmen brach über die MCP-Verbindung ab). Die Dateien
`v01` und `v02` zeigen den Stand vor der Poliersession, `v03` den Stand
danach; sie liegen nebeneinander, damit der Unterschied sichtbar bleibt.
Die `v03`-Dateien sind direkte 2×-Exporte der Bühne (seit die Schraffur ein
einzelner Vektor ist, bricht der Export nicht mehr ab). Der Stand `v03` ist
der aktuelle:

| Datei | Inhalt |
|---|---|
| `2026-09-02-v03-gen-flaeche-1-overview@2x.png` | Gen Fläche 1, Grundrahmen nach der Politur: Kantenspalte 22/928, Quellzeilen 42 im Raster 47 mit Ausblendung der angeschnittenen Zeile, Filterfeld als Einlass, Evidenzgraph mit Log-Achse 20 Hz–20 kHz, LTAS-Kurven BASS/PIANO, Befundzone 180–260 Hz mit Marker und Punkt bei 240 Hz, Kurvenschlüssel oben links, Befund `B3 · 240 Hz · −1.5 dB · Q 1.20`, Aktionsblock als drei Pillen à 30 |
| `2026-09-02-v03-gen-flaeche-2-eq@2x.png` | Gen Fläche 2, Grundrahmen nach der Politur: Log-Achse mit Dekaden- und Zwischenlinien, Skalen `0 dBFS`/`−24`/`−48` links und `+12 dB` bis `−12` rechts rechtsbündig, gerechnete Kurven BASE/DRAFT/REF · PIANO, Verlauf-Füllung bis zum Rasterboden, Griffe auf Frequenz und Gain (B2 aus als gestrichelter Ring), Schutzzone 3–6 kHz auf echter Position, Kurvenschlüssel unter den Ticks |
| `2026-09-02-v03-probeeq-kachel@2x.png` | Probeeq 600×92 nach der Politur: Zeile 1 auf der Bandachse zentriert, `MIX` in Schalengrau, Zeile 2 `REMOTE PROPOSAL READY` und `B3 · 240 Hz · −1.5 dB · Q 1.20` als eine Linie mit festem Abstand |
| `2026-09-02-v03-gen-flaeche-1-befundliste@2x.png` | Gen 1 · Befundliste nach der Politur |
| `2026-09-02-v03-gen-flaeche-1-ohne-befund@2x.png` | Gen 1 · Quelle ohne Befund nach der Politur: KICK gegen MASTER als LTAS-Kurven, keine Zone, Pillen nicht verfügbar |
| `2026-09-02-v03-gen-flaeche-1-filter@2x.png` | Gen 1 · Filtertreffer nach der Politur: Einlass mit Cyansaum im Fokus, keine Ausblendung bei zwei Zeilen |
| `2026-09-02-v03-gen-flaeche-1-draft-offen@2x.png` | Gen 1 · Draft offen nach der Politur |
| `2026-09-02-v03-gen-flaeche-1-kandidat@2x.png` | Gen 1 · Kandidat nach der Politur: `CANDIDATE · OPEN IN EQ →` als Weinpille, Halten nicht verfügbar |
| `2026-09-02-v03-gen-flaeche-1-link-verlust@2x.png` | Gen 1 · Link-Verlust nach der Politur |
| `2026-09-02-v03-gen-flaeche-2-band-panel-dynamic@2x.png` | Gen 2 · Band-Panel Dynamic nach der Politur: Karte folgt dem Griff B3 bei 240 Hz |
| `2026-09-02-v03-gen-flaeche-2-band-panel-grundansicht@2x.png` | Gen 2 · Band-Panel Grundansicht nach der Politur: Griff B4 bei 880 Hz, +0,6 dB wie die Panelwerte, B4 gewählt, B3 behält die Dynamic-Kontur |
| `2026-09-02-v03-gen-flaeche-2-band-panel-typwahl@2x.png` | Gen 2 · Typwahl nach der Politur |
| `2026-09-02-v03-gen-flaeche-2-band-panel-kanalmodus@2x.png` | Gen 2 · Kanalmodus nach der Politur |
| `2026-09-02-v03-gen-flaeche-2-global-offen@2x.png` | Gen 2 · Global offen nach der Politur |
| `2026-09-02-v03-gen-flaeche-2-preset-liste@2x.png` | Gen 2 · Preset-Liste nach der Politur |
| `2026-09-02-v03-gen-flaeche-2-kopie-zielwahl@2x.png` | Gen 2 · Kopie-Zielwahl nach der Politur: Zielleiste auf der Kantenspalte, `CONFIRM COPY` mit Abstand 14 nach der Folgezeile, `CANCEL` rechts bei 928 |
| `2026-09-02-v03-gen-flaeche-2-halten@2x.png` | Gen 2 · Halten nach der Politur: Draft als hörbare helle Kurve mit Saum, Füllung folgt dem Draft, Basis tritt dünn zurück |
| `2026-09-02-v03-gen-flaeche-2-kandidat@2x.png` | Gen 2 · Kandidat nach der Politur |
| `2026-09-02-v03-gen-flaeche-2-bestaetigt@2x.png` | Gen 2 · Bestätigt nach der Politur: keine Draft-Kurve, Schlüssel ohne `DRAFT` |
| `2026-09-02-v03-gen-flaeche-2-link-verlust@2x.png` | Gen 2 · Link-Verlust nach der Politur |
| `2026-09-02-v03-probeeq-eq-aus@2x.png` | Probeeq · EQ aus nach der Politur: `EQ OFF` und `PASSIVE MEASUREMENT · SIGNAL · FRESH 1.2 s` mit festem Abstand |
| `2026-09-02-v03-probeeq-link-verlust@2x.png` | Probeeq · Link-Verlust nach der Politur |
| `2026-09-02-v03-probeeq-kandidat@2x.png` | Probeeq · Kandidat nach der Politur |
| `2026-09-02-v03-probeeq-bestaetigt@2x.png` | Probeeq · Bestätigt nach der Politur |

Die Belege vor der Poliersession:

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
| Gen 1 · Quelle ohne Befund | `6132:182` | `2026-09-02-v02-gen-flaeche-1-ohne-befund@2x.png` | `KICK` gewählt, Referenz `MASTER`, `FRESH · 2.0 s · NO FINDING`, keine Zone im Graph, Kopf `FINDINGS · 0`, `ALL FINDINGS` nicht verfügbar, `NO FINDING`, Halten und Draft nicht verfügbar |
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
| Probeeq · Kandidat | `6132:3293` | `2026-09-02-v02-probeeq-kandidat@2x.png` | `REMOTE CANDIDATE · 8.3 s` mit der laufenden Änderung |
| Probeeq · Bestätigt | `6132:3326` | `2026-09-02-v02-probeeq-bestaetigt@2x.png` | `CONFIRMED · REV 128` mit der bestätigten Änderung |

Panels liegen als Karte über dem Glas (#131619 bei 97 %, Kontur weiß 8 %,
Radius 6 seit der Poliersession, vorher 7; Schatten), wie die Draft-Karte des alten Editors; Tasten darauf
sind Kartenpillen mit Radius 6, Werte sind dunkle Einlässe (#0c0e10), und
Spiegelung und Vignette des Glases bleiben über der Karte. Nicht verfügbar
heißt überall Beschriftung und Fläche auf 45 %.

Bewusste Abweichungen von der Skizze in diesem Schritt (Dichteprüfung des
Wortlauts, keine neuen Entscheide):

- `ALL FINDINGS` bleibt bei geöffneter Liste gedrückt statt in
  `CURRENT FINDING` umbenannt zu werden; die Taste behält 84 px
  (Tasten-Gesetz), die Skizze verbreitert sie.
- Ohne Befund zeigt der Evidenzgraph keine Zone, keinen Marker und keinen
  Punkt, der Befundkopf sagt `FINDINGS · 0` und `ALL FINDINGS` ist nicht
  verfügbar; die Skizze lässt Zone und `FINDING 1 / 3` stehen.
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

## Poliersession (dritter Schritt, 02.09.2026, Abend)

Auftrag und Wortlaut stehen in
[../abnahmen/2026-09-02-wechsel-visuelle-phase-figma.md](../abnahmen/2026-09-02-wechsel-visuelle-phase-figma.md)
(Abschnitt „Poliersession"). Maßstab war der Probeeq-Editor 700×420 der
Materialstudie; das Materialrezept war bereits identisch, der Unterschied
lag in Geometrie, Achsen, Typo-Disziplin und einigen echten Fehlern. Alle
24 Rahmen (drei Grundrahmen, 21 Zustände) liefen in Figma durch dieselben
Routinen, die Zustandsunterschiede blieben erhalten:

- **Echte Fehler behoben:** logarithmische Frequenzachse mit Raster auf
  echten Frequenzen (vorher 20→200 und 200→2k ungleich breit, Rasterlinien
  ohne Bezug); Bass-Kurve fällt zu den Höhen, Analyzer als Spektrum statt
  Blob; Weinfüllung ohne harte Kante 30 px über dem Rasterboden; Griff
  B2 „aus" ohne Durchstreich-Linie; dB-Skalen rechtsbündig (vorher stand die
  0 fünf Pixel neben +6/−6); Halbpixel-Geometrie entfernt; ein Zahlenstand
  für den Draft (B3 · 240 Hz · −1,5 dB · Q 1,20 auf allen drei Apps, vorher
  220 Hz/−2 dB/Q1.2 auf Fläche 1) und „92 %" mit Leerzeichen überall.
- **Feinschliff:** Kantenspalte 22/928 auf Schale und Glas (vorher 14, 20,
  21, 24 links und 935, 939 rechts); Aktionsblock Fläche 1 als drei
  Materialpillen à 30 in der Höhe des Kinns; Grautöne von acht auf feste
  Rollen; Labels auf Glas #6e757d statt #5c6269; Befundzone mit Füllung,
  zwei Haarlinien und 11-px-Punkt statt Strichrahmen und 15-px-Punkt;
  Kurvenschlüssel oben links auf beiden Flächen, die schwebenden Labels
  `BASS`/`PIANO` und die Überschriften `ANALYZER`/`EQ` entfallen;
  Probeeq-Zeile 1 auf der Bandachse zentriert (vorher vier Pixel zu tief),
  Zeile 2 als ein Text mit Mittelpunkten und festem Abstand 14; Filterfeld
  als dunkler Einlass (#0a0c0e, Innenschatten, im Filterzustand Cyansaum);
  Radienfamilie 4 (Felder, Zeilen) / 6 (Karten) / Pille; die angeschnittene
  neunte Quellzeile läuft in eine Ausblendung.
- **Bewusst nicht angefasst:** Materialrezept, Positionen und Tastenhöhen
  der Skizze (30/28/26/20), der `MASTER`-Ring (die Skizze gibt ihn vor),
  die Wortmarke (Verlauf rendert nur in Figma).

Zusätzliche bewusste Abweichungen von der Skizze seit der Poliersession:

- Kurvenformen, Griffpositionen, Zonenbreite und Schutzbereich folgen der
  echten Frequenzachse und den Bandparametern, nicht den illustrativen
  Pfaden der Skizze; die Skizze bleibt Layout-Wahrheit für Zonen, Größen
  und Handgriffe.
- Der Aktionsblock von Fläche 1 hat die Höhe der Kinn-Einsätze; die Skizze
  streckt ihre Zellen mit dem Befundbereich.
- Die Findings von Fläche 1 nennen 240 Hz, −1,5 dB und Q 1,20 wie Fläche 2
  und Probeeq; die Skizze sagt dort noch 220 Hz, −2 dB und Q1.2.

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
