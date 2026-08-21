# Sondenplan — Active-Probe-Editor und Passive-Kachel

Der Auftrag für die nächsten zwei Oberflächen. Entstanden am **2026-08-21** aus
einer Code-Review der bisherigen Arbeit; Prüfbericht:
`abnahmen/2026-08-21-codereview.md`.

Diese Datei ersetzt keine Abnahme und entscheidet nichts. Sie legt hin, was
**gemessen** ist, was **entschieden** ist, und was **der User entscheiden muss**,
bevor gebaut wird. Alle Zahlen darin sind am laufenden Blatt nachgemessen, nicht
übernommen.

---

## 0. Wo wir stehen, in vier Zeilen

| | |
|---|---|
| Main, Wireframe-Ebene | **abgenommen**, Entwurf in `werkzeug/zustaende.html` |
| Active-Probe-Editor 700×420 | **nicht begonnen** — der nächste Schritt |
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
2. **Natürliche Höhen messen**, bevor irgendetwas angeordnet wird
   (Abschnitt 4 nimmt das vorweg, aber am fertigen Inhalt neu messen).
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

### Was es noch gar nicht gibt

**Die Verdeckungsprüfung.** `werkzeug/PRUEFLISTE.md` verlangt sie („Overlays
fallen durch jede Überlaufmessung"), aber sie existiert in **keinem** Blatt als
Code — die Skripte waren sitzungslokal und sind weg. Der Editor braucht sie
dringender als Main: Draft-Kurve über der Summenkurve, Schutzbereichs-Band im
Kurvenfeld, Typ-Auswahl über Bandzeilen. Nachweis, dass sie fehlt: die
markierte Zeile in `zustaende.html` deckte 1 px der Nachbarzeile ab, und keine
Prüfung hat das je gemeldet (Outlines ändern keine Scroll-Maße).

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

| | Höhe | Breite |
|---|---|---|
| Bandzeile, alle 12 Parameter | **20 px** | 536 px (von 664 nutzbar) |
| Bandzeile, nur die 6 EQ-Werte | **20 px** | 262 px |
| Kastenrahmen leer (Kopf + Polster) | 39 px | — |

**Der wichtigste Befund dieser Rechnung: die Höhe wird von der ZAHL DER
GESTAPELTEN ZEILEN gekauft, nicht von der Zahl der Werte darin.** Eine Zeile
ist 20 px hoch, weil die Schrift 11 px hat — ob sechs oder zwölf Werte
darinstehen, ändert nur die Breite. Die Breite ist nicht das Problem: zwölf
Werte brauchen 536 von 664 px, es bleiben 128 px Luft.

### Vier Anordnungen, jede gemessen

| | Bandkasten | Kurvenfeld | px je dB (±12) | ±3-dB-Draft belegt |
|---|---|---|---|---|
| **A** — 8 Zeilen × 12 Parameter | 216 | **103 px** | 4,3 | 26 px |
| **B** — 8 Zeilen × 6 Parameter | 216 | **103 px** | 4,3 | 26 px |
| **C** — 4+4 nebeneinander, 6 Parameter | 120 | **199 px** | 8,3 | 50 px |
| **D** — 1 Band voll + 7 Streifen (gestapelt) | 216 | **103 px** | 4,3 | 26 px |

**A, B und D sind identisch** — genau das ist der Punkt: acht gestapelte Zeilen
kosten 216 px, egal was drinsteht. Nur **C** halbiert den Bandkasten und
verdoppelt das Kurvenfeld, weil es die Zeilen **nebeneinander** legt statt
untereinander.

**Was daraus folgt, bevor der User entscheidet:**

- „Alle acht Bänder ausführlich **und** eine brauchbare Kurve" geht auf dieser
  Fläche nicht. 103 px Kurvenfeld sind 4,3 px je dB; ein ±3-dB-Draft belegt
  darin 26 px — die Höhe eines Knopfes.
- Der Hebel ist **nicht**, Werte je Band wegzulassen (das kauft nur Breite,
  und Breite ist übrig). Der Hebel ist, **weniger Zeilen zu stapeln**:
  nebeneinander legen, oder nur das gewählte Band ausführlich zeigen — und
  dann sind die sieben anderen als **waagerechte** Streifen zu führen, nicht
  als weitere Zeilen.
- Eine einzeilige Bandübersicht scheitert an der **Breite**: acht Streifen mit
  „● 3 · 220 Hz · −3.5 dB" nebeneinander brauchen mehr, als da ist. Ohne
  Einheiten passen sie — aber eine Zahl ohne Einheit sagt auch nichts.

**Offene Produktfrage für den User:** bei Main war klar, wer seine natürliche
Höhe hält — die Karte, weil sie das dauerhafte Ding ist. Beim Editor ist das
offen: die Kurve ist laut Spezifikation „zentrale Anzeige", die Bänder sind
das, was bedient wird. **Wer gibt bei Platzmangel nach?**

### Annahmen, offen genannt

1. Typografie und Polster sind aus Main übernommen. Eine eigene Grammatik
   verschiebt alle Zahlen.
2. Alle Zeilen sind **reine Textzeilen ohne Greif-Affordanz**. Ein Regler oder
   eine Trefferfläche nach Fitts kostet mehr — ein `.wfk` (Knopf) ist gemessen
   23 px hoch. Mit echten Bedienelementen liegt eine Bandzeile eher bei 24 px,
   acht davon bei ~250 px, und der Kurve bliebe unter 70 px.
3. **Kein Global-Bereich, keine Draft-Knopfreihe, kein Undo-Verlauf im Blatt.**
   Kommen sie dauerhaft dazu, schrumpft der Vorrat von 358 deutlich, und
   Anordnung A passt nicht mehr.
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

**Aber:** der Active-Probe-Kasten dieses Blattes ist **kein Modell des
Editors**. Er zeigt eine Kurve und eine Zeile; der Editor hat acht Band-Slots
mit je zwölf Parametern. Das Blatt misst jetzt ehrlich — noch den falschen
Inhalt. **Erster Handgriff der nächsten Runde: den Mock mit echtem Inhalt
füllen, dann messen.** Dasselbe gilt für die Passive-Kachel: 260×84 ist breit
und flach, dort entscheidet die **Breite**, und ein Name wie
„Backing Vox — Insert · POST" ist der Grenzfall.

---

## 6. Die Stolperfallen, benannt

Aus `werkzeug/PRUEFLISTE.md` und den vier Fallen des letzten Sessionendes —
diese treffen den Editor besonders hart:

| Falle | Warum hier schlimmer | Vorsichtsmaßnahme |
|---|---|---|
| **Deckel unter der Inhaltshöhe** | Mains Karte hat 8-px-Zeilen, weil dort **kein Buchstabe** steht. Wer sie als Bandzeile kopiert, deckelt zwölf Werte à 11 px auf 8 px | Jede Bandzeile `min-height:max-content`, nie eine Pixelzahl |
| **Nur die Höhe messen** | Die Passive-Kachel ist breit und flach — dort entscheidet die Breite | `ueberlauf()` aus `zustaende.html` kopieren, **nicht** aus `wireframe-main*.html` |
| **Ein Knopf, der nichts tut** | **40 Werte** (5 Dynamik-Werte × 8 Bänder) sind tot, solange „dynamisch" aus ist. Dazu: Draft ohne Main, jeder Bandparameter bei Bypass, jeder Griff in einem Schutzbereich | Was nichts bewirkt, ist abgeschaltet und zeigt „—". Das Muster steht fertig in `knopfreihe()` |
| **Overlays fallen durch jede Überlaufmessung** | Draft-Kurve über der Summenkurve, Schutzbereichs-Band, Typ-Auswahl über Bandzeilen | Die Prüfung **neu bauen** — sie existiert nirgends — und mit einem künstlichen Fehler gegenprüfen |
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

1. `docs/sondenplan.md` (diese Datei) und
   `abnahmen/2026-08-21-codereview.md` lesen.
2. `/interview struktur` für den Active-Probe-Editor.
3. Die Aufgabenliste aus Abschnitt 3 vorlegen — **mit** den zwei offenen Fragen.
4. Den Active-Probe-Mock in `formfaktor.html` mit echtem Inhalt füllen und
   700×420 neu vermessen.
5. Erst dann Varianten bauen, unterschieden in der Disclosure-Mechanik.
