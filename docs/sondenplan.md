# Sondenplan — Active-Probe-Editor und Passive-Kachel

Der Auftrag für die nächsten zwei Oberflächen. Entstanden am **2026-08-21** aus
einer Code-Review der bisherigen Arbeit; Prüfbericht:
`abnahmen/2026-08-21-codereview.md`.

Diese Datei ersetzt keine Abnahme und entscheidet nichts. Sie legt hin, was
**gemessen** ist, was **entschieden** ist, und was **der User entscheiden muss**,
bevor gebaut wird.

> **⚠ Berichtigt 2026-08-21.** Hier stand: „Alle Zahlen darin sind am laufenden
> Blatt nachgemessen, nicht übernommen." Das stimmte im Wortlaut und war
> trotzdem irreführend: das Blatt, an dem gemessen wurde, war **sitzungslokal
> und ist weg** — genau der Verlust, den `werkzeug/PRUEFLISTE.md` schon einmal
> beklagt hat. Nachmessen, was niemand nachmessen kann, ist kein Nachmessen.
> Seit dem 2026-08-21 steht das Blatt: **`werkzeug/sonde-messung.html`**.
> Beim ersten Lauf waren von **19 nachprüfbaren Angaben** in Abschnitt 4
> **10 falsch** — darunter jede einzelne zu Anordnung A und die Behauptung,
> A, B und D seien identisch. Richtig waren vor allem die Vorratsrechnung
> (682×402, Streifen 28, 358 px) und die Zahlen zu Anordnung C. Alles steht
> unten berichtigt, mit dem alten Wert daneben.

---

## 0. Wo wir stehen, in vier Zeilen

| | |
|---|---|
| Main, Wireframe-Ebene | **abgenommen**, Entwurf in `werkzeug/zustaende.html` |
| Active-Probe-Editor 700×420 | **Klemme gemessen** (`werkzeug/sonde-messung.html`, 2026-08-21) — Aufgabenliste und Wireframe offen |
| Passive-Kachel 260×84 | **nicht begonnen** — danach |
| Designvertrag (Phase 0) | **fehlt** — die Schleuse vor `prototyp/` ist zu |

Der fehlende Designvertrag hindert diese Runde **nicht**: gearbeitet wird in
`werkzeug/`, nicht in `prototyp/`. Er wird gebraucht, bevor die erste
Prototyp-Datei entsteht, und spätestens vor Phase 2 (Stilprobe) — dort fehlen
sonst Erfolgskriterien, Antireferenzen und der Grad der kreativen Autorenschaft.

---

## 1. Die Reihenfolge ist vorgeschrieben

`docs/DESIGN-GESETZE.md`, verbindliche Folge 1: **ohne Aufgabenliste nach
Häufigkeit kein Wireframe.** Und das Muster, das bei Main getragen hat:

1. **Aufgabenliste nach Häufigkeit** — vom User bestätigen lassen (Abschnitt 3).
2. **Natürliche Höhen messen**, bevor irgendetwas angeordnet wird.
   ✓ **Erledigt 2026-08-21**: `werkzeug/sonde-messung.html` misst jeden
   Baustein einzeln und jede Anordnung real gebaut. Wer daran etwas ändert,
   misst dort nach — nicht in dieser Datei.
3. **Erst dann Varianten** — und die unterscheiden sich in der
   **Disclosure-Mechanik**, nicht in der Kastenanordnung.

Vorher `/interview struktur` für den Editor.

---

## 2. Was kopiert wird — und woraus

**Kopiert wird ausschließlich aus `werkzeug/zustaende.html`.** Es ist bei jeder
gemeinsamen Funktion die neueste Fassung. Die drei eingefrorenen Belegblätter
enthalten ältere, teils blinde Fassungen derselben Namen; sie werden gelesen,
nicht kopiert.

### Der Kern, der trägt

| Was | Zweck | Übernehmen? |
|---|---|---|
| `listenEhrlich` / `einmalEhrlich` | Zeigt nur, was ganz hineinpasst; schreibt die **gemessene** Zahl `n/gesamt` in den Kopf und „N more not shown" darunter | **ja, unverändert.** 8 Bänder und ein Undo-Verlauf sind Listen wie jede andere |
| `listenZuruecksetzen` | Macht alle Listen vor dem Messen wieder voll | **ja, Pflicht.** Ohne sie misst der zweite Durchgang Gekürztes als „natürliche Höhe" — die Ratsche |
| `aufteilen` | Misst einmal die ungekürzten Höhen: **Überschuss teilen, Mangel nicht** | **ja, mit Umbenennung.** Greift hart auf `.karte`/`.arbeit` zu |
| `ueberlauf` | Rekursiv über **alle** Nachfahren, **beide** Richtungen | **ja — und nur diese Fassung.** Die in `wireframe-main*.html` misst nur die Höhe |
| `LAGE` · `griff` · `verdrahten` · `alleNeu` | Echtes `pointerdown`/`pointerup`-Halten statt Schalter, mit `pointerleave`/`pointercancel` | **ja, das wertvollste Einzelstück.** Die Spezifikation verlangt für Draft/Preview wörtlich diese Mechanik: „gehalten = hörbar, losgelassen = weg" |
| `kasten` · `liste` | Kasten mit Kopf, Zählerfeld, `data-baustein` | **ja, 1:1** |
| `vertrag` + `messbuehne` + `vertragCache` | Misst, welche Bausteine in **allen** Arbeitsschritten dastehen | **Mechanismus ja, Inhalt nein.** Er hängt an `ZUSTAENDE` (Mains drei Schritte). Bleiben die stehen, misst er stumm Unsinn |
| `BELEG` / `AUFGABEN` / `belegFuer` | Jeder Baustein trägt Handgriff + Spezifikationsstelle; ohne Beleg wird er rot | **Gerüst ja, Inhalt LEEREN.** Alle Einträge sind Main-Bausteine. Nicht anpassen — leeren, sonst nennt jemand einen Editor-Baustein „Befundliste", damit das Rot verschwindet |
| `knopf` · `band` · `ablehnen` · `waehlZeile` · `zeichnen` · `kachel` | Werkzeug-Chrome | **ja** |

### Was ausdrücklich NICHT taugt

- **`heatmap()`** — ihre ganze Zustandsgrammatik ist *Frische* (`zust-alt` hohl,
  `zust-weg` gestrichelt). Ein EQ-Band hat keine Frische. Es braucht Nummer,
  Typ, zwölf einstellbare Werte, an/aus, dyn an/aus, geschützt ja/nein.
- **`SOURCES` · `SZENARIEN` · `szenarioSetzen`** — sechs Szenarien auf der Achse
  *Quellenfrische*. Der Editor hat eine andere Achse: Band an/aus · dyn an/aus ·
  Draft gehalten/angewandt/weg · verbunden/getrennt · gekoppelt · Bypass ·
  Schutzbereich getroffen.
- **`statusText` / `messText` / `degText`** — 40 Zeilen für sechzehn Quellen.
  Der Editor braucht **zwei** Angaben: verbunden/getrennt, gekoppelt ja/nein.
  Die *Regel* („nur die Ausnahme spricht") ist übertragbar, der Code nicht.
- **`befunde` · `advisor` · `vergleich` · `detail`** — kein Gegenstück in
  Abschnitt 2 der Spezifikation. Übertragbar sind zwei **Muster**: „einer offen,
  der Rest als benannte Zeile" (für den Undo-Verlauf) und die ehrliche Liste.
- **Der Reiterstreifen `EBENEN`** — die vier Bedienebenen stehen in der
  Spezifikation unter *Main*, nicht unter Abschnitt 2. Der Editor kennt einen
  flüchtigen Draft-Zustand und bewusste Klicks; das ist nicht dieselbe
  Vierteilung. Die **Haltemechanik** kommt trotzdem mit.

### Was es bis zum 2026-08-21 gar nicht gab

**Die Verdeckungsprüfung.** `werkzeug/PRUEFLISTE.md` verlangt sie seit dem
20.08. („Overlays fallen durch jede Überlaufmessung"), sie existierte in
**keinem** Blatt als Code. ✓ Sie steht jetzt in `werkzeug/sonde-messung.html`
als `verdeckung()` und meldet zwei Sorten getrennt:

- **Randfarbe** — `outline`/`box-shadow` reicht über die Layoutbox hinaus und
  landet auf fremdem Text. Das ist der Fall aus `zustaende.html`: die markierte
  Zeile deckte 1 px der Nachbarzeile ab, und keine Prüfung hat es je gemeldet,
  weil Outlines keine Scroll-Maße ändern.
- **Auflage** — ein absolut gesetztes Element liegt auf fremdem Text. Wer
  absichtlich verdeckt, sagt es mit `data-deckt="absicht"`; erklärte Absicht
  ist erlaubt, unerklärte Verdeckung ist der Befund.

**Sie hat sich im ersten Lauf sofort bezahlt gemacht.** Gefunden: die Regel
`.anordnung .nr{position:absolute}` (Nummernmarke der Kachel im Chrome) griff
auch auf `.slotz .nr` (Slot-Nummer im Spezimen) — **alle acht Slot-Nummern
lagen übereinander in der linken oberen Ecke**, und in den Bandzeilen fehlte
die Nummer. Kein Überlaufmaß kann so etwas sehen.

**Zwei Fallen der Prüfung selbst**, beide zuerst zugeschlagen:

1. **Ohne Clipping meldet sie Gespenster.** `getBoundingClientRect()` kennt
   kein `overflow:hidden`. Eine Achsenbeschriftung ragte geometrisch 10,9 px
   in die Bandliste, sichtbar war davon nichts. Sie rechnet jetzt jedes
   Rechteck gegen seine beschneidenden Vorfahren.
2. **Ein globaler Höchstwert deckt eine stumme Sorte.** Solange nur die
   schlimmste Verdeckung gemeldet wurde, verbarg die (falsche) Auflage von
   10,9 px, dass die Randfarben-Sorte überhaupt nichts fand. Sie meldet
   jetzt **je Sorte** — dieselbe Regel wie PRUEFLISTE 7.2 („je Kachel prüfen,
   nicht global").

### Beim Kopieren zwingend zu ändern

`.rahmen{width:calc(760px * var(--s));height:calc(430px * var(--s))}` ·
`.wf{width:760px;height:430px}` · `t.style.width = (gross ? 760 : 440)` ·
der Text im Prüfband · `BELEG` leeren · `ZUSTAENDE` durch die Arbeitsschritte
des Editors ersetzen. Die `messbuehne` (800×500) reicht für 700×420 unverändert.

---

## 3. Aufgabenliste nach Häufigkeit — ENTWURF, dem User vorzulegen

> **Nicht abgenommen. Kein Wireframe auf dieser Grundlage, bevor der User sie
> beurteilt hat.** Die *Handgriffe* sind vollständig aus
> `docs/oberflaechen-spezifikation.md` Abschnitt 2 abgeleitet und belegt. Die
> *Häufigkeit* steht dort **nirgends** — jede Zeile der Spalte ist eine
> Produktaussage und gehört dem User. Geteilt wird nach Häufigkeit der Aufgabe,
> nicht nach Können des Nutzers.

| # | Handgriff | Häufigkeit (Entwurf) | Braucht |
|---|---|---|---|
| 1 | **Die Kurve lesen** — was macht dieser EQ gerade? | oft, ohne Absicht | Summenkurve + Einzelbänder, Schutzbereiche eingezeichnet |
| 2 | **Ein Band greifen, Frequenz/Gain ziehen** | oft — der Kernakt | Frequenz, Gain (±12 dB) |
| 3 | **Q verstellen** | oft — folgt fast jedem Griff aus 2 | Q |
| 4 | **Ein Band an-/ausschalten** (der Einzel-A/B) | oft | an/aus je Band |
| 5 | **Bypass des ganzen Geräts** | oft | Bypass |
| 6 | **Einen Vorschlag vom Main vorhören** (halten) | oft, sobald Main läuft | Draft-Zustand, ±3 dB statt ±12 |
| 7 | **Vorschlag anwenden oder verwerfen** | oft — folgt jedem Vorhören | zwei bewusste Klicks |
| 8 | **Bandtyp wechseln** | gelegentlich — meist einmal je Band | Typ |
| 9 | **Kanalmodus wechseln** | gelegentlich | Stereo / L-R / M-S |
| 10 | **Dynamik einschalten und einstellen** | gelegentlich — nie an allen acht | dyn an/aus + Schwelle, Bereich, Attack, Hold, Release |
| 11 | **Einen Schritt zurücknehmen** | gelegentlich | Undo-Verlauf |
| 12 | **Ein-/Ausgangs-Trim setzen** | gelegentlich — meist einmal je Sitzung | In-Trim, Out-Trim |
| 13 | **Stereobreite / Mono-Bass setzen** | selten — meist einmal je Projekt | Breite, Mono-Bass-Grenze |
| 14 | **Neutralisieren** | selten | — |
| 15 | **Schutzbereich setzen oder aufheben** | selten | gesperrter Bereich |
| 16 | **Verbindung und Kopplung prüfen** | ständig nebenbei, nie das Ziel | verbunden, gekoppelt |

**Was der Entwurf bereits behauptet, wenn er stimmt:** die Handgriffe 1–7 sind
„oft" und betreffen **ein** Band plus die Kurve plus zwei Knöpfe. Die
Handgriffe 8–15 sind „gelegentlich/selten" und enthalten **alle sechs
Dynamik-Werte**. Die Disclosure-Trennlinie liefe damit **quer durch jedes
Band**, nicht zwischen Bändern.

**Zwei Fragen, die nur der User beantworten kann:**

1. Ist **Bypass** wirklich so häufig wie das Ziehen eines Bandes? Davon hängt
   ab, ob er nach Fitts groß und nah sein muss.
2. Ist **Draft/Preview vom Main** häufig oder ein Sonderfall? Die Spezifikation
   nennt den Editor ausdrücklich eigenständig — „muss allein funktionieren,
   auch ohne Main". Ist Draft selten, gehört er nicht dauerhaft aufs Blatt.

---

## 4. Die Klemme, vorgerechnet

Gemessen am 2026-08-21 in der Typo-Grammatik von `zustaende.html`
(12px/1.4 Segoe UI, Werte 11 px), auf einem echten 700×420-Blatt.

### Der Vorrat

| | Main 760×430 | **Editor 700×420** |
|---|---|---|
| innen, nach 8 px Polster | 742 × 412 | **682 × 402** |
| Streifen (dauerhaft) | 28 | **28** |
| zwei Lücken à 8 px | 16 | **16** |
| **bleibt für zwei Bereiche** | **368** | **358** |

Der Editor hat **10 px weniger Höhe und 60 px weniger Breite** als Main — für
**101 einstellbare Werte** (8 Bänder × 12 Parameter = 96, plus 5 globale)
statt für sechzehn reine Anzeigezeilen.

### Was eine Bandzeile kostet

> **⚠ Nachgemessen am 2026-08-21 an `werkzeug/sonde-messung.html`.** Die Zahlen
> dieses Abschnitts stammten von einem 700×420-Blatt, das es nicht mehr gab —
> sitzungslokal gemessen, genau wie die Playwright-Skripte, deren Verlust
> `werkzeug/PRUEFLISTE.md` beklagt. Sie sind jetzt reproduzierbar, und **fünf
> von neun waren falsch.** Die alten Werte stehen in Klammern dahinter.

| | Höhe | Breite |
|---|---|---|
| SLOT-Zeile, alle 12 Parameter | **15,4 px** *(hieß 20)* | 564 px von 682 *(hieß 536 von 664)* |
| SLOT-Zeile, nur die 6 EQ-Werte | **15,4 px** *(hieß 20)* | 232 px *(hieß 262)* |
| SLOT-Zeile, **längste** Werte | **15,4 px** | **564 px** — identisch |
| acht SLOT-Zeilen gestapelt (mit Fugen) | **137,1 px** | — |
| Kastenrahmen leer (Kopf + Polster) | **38,6 px** *(hieß 39)* ✓ | 105 px |
| Bandkasten 8 × 12 Parameter, mit Rahmen | **175,7 px** *(hieß 216)* | 584 px |
| Bandkasten 8 × 6 EQ-Werte, mit Rahmen | **175,7 px** — identisch | 252 px |
| Statuszeile · Global · Draft · Undo-Zeile | 28 · 33,3 · 33,3 · 33,3 px | — |
| Undo als Liste (5 Schritte) | **203,3 px** | 224,8 px |

**Der wichtigste Befund steht — und ist jetzt belegt: die Höhe wird von der
ZAHL DER GESTAPELTEN ZEILEN gekauft, nicht von der Zahl der Werte darin.**
Acht Zeilen kosten 137,1 px, ob sechs oder zwölf Werte darinstehen; nur die
Breite ändert sich (564 gegen 232). Der Bandkasten ist in beiden Fällen auf
das Zehntel gleich hoch.

**Neu und nicht vorhergesehen: auch die längsten Werte kosten keine Breite
mehr.** Die Spalten-Mindestbreiten fassen `Notch · Q 24.00 · −12.0 dB ·
−60.0 dB · 3000 ms` ohne zu wachsen. Reiner Text ohne Spaltenausrichtung:
**376 px** (realistisch) gegen **448,5 px** (längste) — die restlichen ~120 px
der 564 sind der Preis dafür, dass die Spalten untereinander fluchten.

### Die Nachzählung, damit „10 von 19" nicht selbst eine Behauptung ist

**Richtig (9):** Blatt innen 682×402 · Statuszeile 28 px · bleibt 358 px ·
Kastenrahmen leer 39 px *(gemessen 38,6)* · „B ist identisch mit A" ·
C Kurvenfeld 199 *(197)* · C px je dB 8,3 *(8,2)* · C Draft 50 px *(49)* ·
„acht einzeilige Streifen sind zu breit".

**Falsch (10):** SLOT-Zeile 12 Parameter 20 px hoch *(15,4)* · 536 px breit
*(564)* · SLOT-Zeile 6 Werte 20 px hoch *(15,4)* · 262 px breit *(232)* ·
A Bandkasten 216 *(176)* · A Kurvenfeld 103 *(127)* · A px je dB 4,3 *(5,3)* ·
A Draft 26 px *(32)* · C Bandkasten 120 *(106)* · „D ist identisch mit A"
*(D ist 111/192, nicht 216/103)*.

**Das Muster dahinter:** falsch war alles, was von der Zeilenhöhe abhängt —
sie war mit 20 px statt 15,4 um 30 % zu hoch angesetzt. Richtig war alles,
was aus dem Fensterrahmen folgt. Wer die Grundeinheit falsch schätzt, bekommt
jede darauf gebaute Zahl falsch; die Rahmenrechnung merkt davon nichts.

### Fünf Anordnungen, jede am laufenden Blatt gemessen

Unter der **Annahme dieses Abschnitts** (Global, Draft und Undo stehen NICHT
dauerhaft auf dem Blatt → 358 px für Kurve + Bänder, siehe „Annahmen" unten),
Bänder halten ihre natürliche Höhe:

| | Bandkasten | Kurvenfeld | px je dB (±12) | ±3-dB-Draft belegt |
|---|---|---|---|---|
| **A** — 8 Zeilen × 12 Parameter | **176** *(hieß 216)* | **127 px** *(hieß 103)* | 5,3 *(hieß 4,3)* | 32 px *(hieß 26)* |
| **B** — 8 Zeilen × 6 Parameter | **176** | **127 px** | 5,3 | 32 px |
| **C** — 4+4 nebeneinander | **106** *(hieß 120)* | **197 px** *(hieß 199)* ✓ | 8,2 *(hieß 8,3)* ✓ | 49 px *(hieß 50)* ✓ |
| **D** — 1 Slot voll + 7 **waagerechte** Streifen | **111** *(hieß 216)* | **192 px** *(hieß 103)* | 8,0 *(hieß 4,3)* | 48 px *(hieß 26)* |
| **E** — nur die Kurve, Werte auf Abruf | 0 | **311 px** | 13,0 | 78 px |

> **⚠ Die Behauptung „A, B und D sind identisch" war falsch.** A und B sind es
> (beide 176/127). **D nicht** — 1 voller Slot plus 7 waagerechte Streifen sind
> **zwei** gestapelte Zeilen, nicht acht, und liegen damit fast gleichauf mit C.
> Die Tabelle widersprach dem eigenen Fließtext direkt darunter, der den Hebel
> korrekt benannte („die sieben anderen als **waagerechte** Streifen … nicht
> als weitere Zeilen"). Die Tabelle war falsch, der Text richtig.

**E ist die Obergrenze**, nicht ein Vorschlag: 311 px sind alles, was auf
dieser Fläche für eine Kurve überhaupt zu holen ist.

**Was daraus folgt, bevor der User entscheidet:**

- „Alle acht Bänder ausführlich **und** eine brauchbare Kurve" bleibt eng, ist
  aber weniger aussichtslos als angesetzt: 127 px statt 103, und ein
  ±3-dB-Draft belegt darin 32 px statt 26.
- Der Hebel ist **nicht**, Werte je Band wegzulassen — das kauft nur Breite,
  und Breite ist übrig (A und B haben dieselbe Höhe). Der Hebel ist,
  **weniger Zeilen zu stapeln**: nebeneinander (C) oder ein Slot voll plus
  waagerechte Streifen (D). Beide holen rund **65 px** mehr Kurve.
- Eine **einzeilige** Bandübersicht scheitert wirklich an der Breite —
  jetzt gemessen statt behauptet: acht Streifen `● 3 · 220 Hz · −2.0 dB`
  nebeneinander brauchen **830,5 px**, verfügbar sind **682**. Fehlbetrag
  **149 px**. Die **dreizeilige** Streifenform passt dagegen mit 394,5 px
  bequem — sie kostet dafür 52 px Höhe statt 22.

### Der Preis der drei dauerhaften Zeilen — gemessen, nicht angenommen

Annahme 3 unten sagt: Global, Draft und Undo waren nicht eingerechnet. Jetzt
sind sie es, und der Preis ist hoch. Alle drei dauerhaft auf dem Blatt:

| | ohne Global/Draft/Undo | mit allen dreien |
|---|---|---|
| bleibt für Kurve + Bänder | **358 px** | **235 px** |
| A · Kurvenfeld | 127 px | **1 px** |
| A · ±3-dB-Draft | 32 px | **0 px** |
| C · Kurvenfeld | 197 px | 71 px |
| D · Kurvenfeld | 192 px | 66 px |

> **Korrigiert am 2026-08-21 (Codex-Befund 7).** Alle Kurvenzahlen dieses
> Abschnitts lagen um **2 px zu hoch**: gemessen wurde `offsetHeight`, und
> darin stecken die beiden Rahmenpixel von `.kurve`, auf denen nichts
> gezeichnet wird. Jetzt kommt die Zahl aus `clientHeight` — A 3→**1**,
> C 73→**71**, D 68→**66**, E 187→**185**, Formfaktor Active 7→**5**. Der
> Bandkasten behält bewusst `offsetHeight`: seine Zahl ist ein Platzbedarf im
> Layout, und dort zählt der Rahmen mit. Das Urteil „unbrauchbar" wird durch
> die Korrektur nicht weicher, sondern härter.

Bei **1 px Kurvenfeld** passen nicht einmal die Achsenbeschriftungen
`+12 dB / −12 dB` hinein — das Blatt meldet dort 14 px Überlauf. Dasselbe zeigt
`werkzeug/formfaktor.html` jetzt 1:1 über dem nachgestellten FL-Bildschirm:
**„700×420 — passt, aber der Kurve bleiben 5 px (0.2 px je dB)"**. Ein Fenster
kann „passen" und trotzdem unbrauchbar sein; eine Überlaufmessung sieht das
grundsätzlich nicht, sie kennt nur abgeschnitten/nicht abgeschnitten.

### Wer gibt bei Platzmangel nach? — beide Antworten vorgerechnet

Bei Main war klar, wer seine natürliche Höhe hält: die Karte, weil sie das
dauerhafte Ding ist. Beim Editor ist das **offen** — die Kurve ist laut
Spezifikation „zentrale Anzeige", die Bänder sind das, was bedient wird.
Das Blatt entscheidet nichts; es rechnet beide Antworten vor (mit allen drei
Dauerzeilen, Kurven-Mindesthöhe 140 px):

| | Kurve gibt nach | Bänder geben nach |
|---|---|---|
| **A** — 8 × 12 | 1 px Kurve · **8/8 Bänder** · 14 px Überlauf | 83 px Kurve · **2/8 Bänder** |
| **C** — 4+4 | 71 px Kurve · 8/8 Bänder | 83 px Kurve · **4/8 Bänder** |
| **D** — 1+7 Streifen | 66 px Kurve · alle sichtbar | 83 px Kurve · **16 px Überlauf** |

**Das ist die Entscheidung, in Zahlen:** entweder alle acht Bänder und eine
Kurve, die keine ist — oder eine ablesbare Kurve und zwei bis vier Bänder.
Ein dritter Weg existiert nur über die Dauerzeilen: wer Global, Draft oder
Undo einen Schritt entfernt, kauft bis zu 123 px zurück.

### Annahmen, offen genannt

1. Typografie und Polster sind aus Main übernommen. Eine eigene Grammatik
   verschiebt alle Zahlen.
2. Alle Zeilen sind **reine Textzeilen ohne Greif-Affordanz** — das gilt
   weiterhin und ist die wichtigste offene Annahme. Eine SLOT-Zeile misst
   15,4 px, weil sie nichts als Text ist. Mit echten Bedienelementen (ein
   `.wfk`-Knopf ist gemessen 23 px hoch) läge sie eher bei 24 px, acht davon
   bei ~190 px statt 137, und alle Kurvenzahlen oben fielen um rund 55 px.
   **Diese Messung ist noch nicht gemacht** — sie gehört in die Runde, in der
   entschieden wird, wie ein Band gegriffen wird.
3. **Kein Global-Bereich, keine Draft-Knopfreihe, kein Undo-Verlauf im Blatt.**
   ✓ **Jetzt gemessen** statt angenommen: alle drei dauerhaft kosten
   **123 px** — der Vorrat fällt von 358 auf 235, und Anordnung A behält
   1 px Kurvenfeld. Siehe „Der Preis der drei dauerhaften Zeilen" oben.
   Am Blatt umschaltbar.
4. Gerechnet ist der **schlimme Fall**: alle 8 Bänder aktiv, alle mit Dynamik,
   längste Werte. Der Schönfall kostet die Hälfte — und ist als Grundlage
   wertlos.

---

## 5. Das Messgerät ist repariert, aber sein Inhalt ist es nicht

`werkzeug/formfaktor.html` war in vier Punkten blind und ist es nicht mehr:
es misst jetzt rekursiv über alle Nachfahren, in **beide** Richtungen, und auf
**jedem** Fenster — auch auf Active-Probe und Passive-Kachel, die vorher gar
nicht gemessen wurden. Belege in `abnahmen/2026-08-21-codereview.md`.

**Was es dort sofort meldet:** Active-Probe bei 700×420 läuft **24 px** über,
die Passive-Kachel bei 260×84 **3 px**.

> **✓ Erledigt am 2026-08-21.** Beide Fenster tragen jetzt echten Inhalt.
> Der Active-Probe-Kasten zeigt 8 Slots × 12 Parameter, Kurve, Global, Draft,
> Undo und Statuszeile — dieselben Werte wie `werkzeug/sonde-messung.html`.
> Die Passive-Kachel steht im **Grenzfall** („Backing Vox — Bus 12 · supports ·
> Insert · POST · stale 40 s · Duplicate instance on this bus") statt im
> Schönfall („Piano Bus").

**Was die Messung am gefüllten Blatt sagt:**

| | gemessen 2026-08-21 |
|---|---|
| Active-Probe 700×420 | **0 px Überlauf** — aber der Kurve bleiben **5 px** (0,2 px je dB) |
| Passive-Kachel 260×84 | **23 px Überlauf, rein vertikal** (0 px in der Breite; vorher 3 px am Schönfall) |

Die alte Zahl „Active-Probe läuft 24 px über" galt für den alten Mock (eine
Kurve, eine Zeile) und ist damit gegenstandslos. Die neue Zahl ist die
wichtigere Lehre: **das Fenster passt, und trotzdem ist es unbrauchbar.**
Eine Überlaufmessung kennt nur abgeschnitten/nicht abgeschnitten; ob das, was
nicht abgeschnitten ist, noch etwas zeigt, muss ein Blatt eigens sagen. Der
Fenstertitel sagt es jetzt.

---

## 6. Die Stolperfallen, benannt

Aus `werkzeug/PRUEFLISTE.md` und den vier Fallen des letzten Sessionendes —
diese treffen den Editor besonders hart:

| Falle | Warum hier schlimmer | Vorsichtsmaßnahme |
|---|---|---|
| **Deckel unter der Inhaltshöhe** | Mains Karte hat 8-px-Zeilen, weil dort **kein Buchstabe** steht. Wer sie als Bandzeile kopiert, deckelt zwölf Werte à 11 px auf 8 px | Jede Bandzeile `min-height:max-content`, nie eine Pixelzahl |
| **Nur die Höhe messen** | Die Passive-Kachel ist breit und flach — dort entscheidet die Breite | `ueberlauf()` aus `zustaende.html` kopieren, **nicht** aus `wireframe-main*.html` |
| **Ein Knopf, der nichts tut** | **40 Werte** (5 Dynamik-Werte × 8 Bänder) sind tot, solange „dynamisch" aus ist. Dazu: Draft ohne Main, jeder Bandparameter bei Bypass, jeder Griff in einem Schutzbereich | Was nichts bewirkt, ist abgeschaltet und zeigt „—". Das Muster steht fertig in `knopfreihe()` |
| **Overlays fallen durch jede Überlaufmessung** | Draft-Kurve über der Summenkurve, Schutzbereichs-Band, Typ-Auswahl über Bandzeilen | ✓ **gebaut** als `verdeckung()` in `sonde-messung.html`, mit Gegenprobe für **beide** Sorten (`sondenprobe.mjs gegenprobe-verdeckung`) |
| **`getBoundingClientRect()` liefert die TRANSFORMIERTE Größe** | Die Kacheln stehen unter `transform:scale()`. Alle Anordnungszahlen waren um 48 % zu klein — 176 px Bandkasten wurden als 92 gemeldet | `offsetHeight` für Layoutmaße. Und: die Sonde prüft eigens, dass eine Zahl sich **nicht** ändert, wenn der Maßstab sich ändert |
| **Ein Gate, das denselben Fehler macht wie das Blatt, bestätigt ihn** | Die erste Sonde verglich „angezeigt" gegen `getBoundingClientRect()` — beide Seiten falsch, Ergebnis „0 Abweichungen" | Jede Prüfung braucht einen **zweiten, unabhängigen** Weg zur Zahl: Inhalt ändern und verlangen, dass die Zahl mitwandert |
| **Werkzeug-Chrome und Spezimen teilen sich einen Klassennamen** | `.nr` hieß Kachel-Nummer *und* Slot-Nummer; der Nachfahren-Selektor traf beide | Chrome-Klassen mit eigenem Präfix und **direktem Kindselektor** (`.anordnung > .knr`) |
| **Zwei Begriffe unter einem Namen** | `BAND` heißt in `zustaende.html` die **Frequenzspalte** der Heatmap, im Editor der **Slot 1–8**. Dazu „Gain ±12" (manuell) gegen „Gain ±3" (Draft) und „Bypass" (Gerät) gegen „an/aus" (Band) | Namen VOR der ersten Zeile festlegen; `BAND` beim Kopieren umbenennen |
| **Im Grenzfall prüfen** | Der Grenzfall ist: 8 Bänder aktiv **und** alle dynamisch **und** getrennt vom Main **und** Bypass an **und** ein Schutzbereich getroffen | Dieser Fall ist die **Vorgabe** der Messung, nicht der Sonderfall |
| **Was gemessen wird, vorher zurücksetzen** | Die Ratsche kommt zurück, sobald `listenZuruecksetzen()` fehlt — und in `wireframe-main.html` fehlt sie | Beim Kopieren prüfen, dass sie mitkommt |
| **Beim Aufräumen zu viel mitnehmen** | Aus 1200 Zeilen Main wird ein Editor herausgeschnitten | Nach jedem Löschen: Syntax prüfen, Blatt zeichnen lassen, und einen **Fingerabdruck** vorher/nachher vergleichen (Verfahren siehe Prüfbericht) |
| **Nummernverweise** | Acht Bänder, zwölf Parameter, mehrere Anordnungen — „nimm 2 und 3" ist hier vierfach mehrdeutig | Bei jedem Nummernverweis zurückfragen |
| **Sekundärquellen glauben** | Bereits zweimal eingetreten: „370 px" (sind 368) und „zehn Parameter" (sind zwölf) — beides im Auftrag für genau diese Runde | Jede Zahl vor Gebrauch nachmessen, auch die aus dem eigenen Abnahmetext |

---

## 7. Was Main noch fehlt (kein Hindernis für die Sonden)

Aus der Deckungsprüfung gegen `docs/oberflaechen-spezifikation.md`. Diese Punkte
blockieren die Sonden **nicht**, müssen aber vor Phase 2 geklärt werden — Belege
und Zeilennummern in `abnahmen/2026-08-21-codereview.md`:

- Am Befund fehlen **Begründung**, **Alternativursachen** und **billigster
  nächster Beweisschritt**; die Beobachtung steht nur beim geöffneten Befund.
- **„keine Änderung empfohlen"** existiert nicht — der Assistent kann raten,
  nie abraten. Die Spezifikation nennt es ein vollwertiges Ergebnis.
- **Suche, Pinning, Fokusgruppen** und die **musikalischen Rollen** haben keinen
  Ort und keine benannte Tür.
- **Name und Rolle** einer Quelle sind in der Karte nicht ablesbar; im Streifen
  nur, solange es ≤2 Abweichungen je Sorte gibt.
- **„Veraltet" löscht die Energie**, statt sie zu altern — hohl liest sich als
  „leer". Das ist eine **Gestaltungsfrage für den User**, keine Reparatur: er
  hat die Variante mit diesem Bild gewählt.

---

## 8. Der erste Handgriff der nächsten Session

**Erledigt am 2026-08-21** (Messrunde, keine Gestaltungsrunde):

- ✓ `werkzeug/sonde-messung.html` — die Klemme ist messbar statt behauptet.
  Alle Zahlen aus Abschnitt 4 sind dort nachrechenbar; fünf davon waren falsch
  und stehen berichtigt.
- ✓ Der Active-Probe-Mock in `formfaktor.html` trägt echten Inhalt; die
  Passive-Kachel steht im Grenzfall (Abschnitt 5).
- ✓ Die **Verdeckungsprüfung** aus `PRUEFLISTE.md` 2.3 existiert — sie hat im
  ersten Lauf einen echten Fehler gefunden (siehe Abschnitt 6).
- ✓ `werkzeug/pruefung/sondenprobe.mjs` — zwölf Proben, darunter zwei
  Gegenproben und ein Durchlauf über alle sechzehn Grenzfälle der
  Ehrlichkeitsschalter.

**Offen, in dieser Reihenfolge:**

1. `/interview struktur` für den Active-Probe-Editor.
2. Die Aufgabenliste aus Abschnitt 3 vorlegen — **mit** den zwei offenen
   Fragen, und **mit** der dritten, die die Messung erzwungen hat: *wer gibt
   bei Platzmangel nach, und dürfen Global, Draft und Undo dauerhaft
   dastehen?* Ohne diese Antworten kein Wireframe
   (`docs/DESIGN-GESETZE.md`, verbindliche Folge 1).
3. Erst dann Varianten bauen, unterschieden in der Disclosure-Mechanik.
