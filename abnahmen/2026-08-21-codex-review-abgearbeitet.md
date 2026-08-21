# 2026-08-21 — Codex-Review abgearbeitet: 10 Befunde geprüft, 14 Punkte erledigt

Nachfolger von [2026-08-21-offen-codex-review.md](2026-08-21-offen-codex-review.md)
(Übersicht) und [2026-08-21-codex-review-volltext.md](2026-08-21-codex-review-volltext.md)
(Volltext, unverändert). **Kein Befund ist still verschwunden** — jeder steht
unten mit Zustand, Beleg und Commit.

Geprüft wurde gegen `34ba0b1`; die vier Quelldateien waren dort byte-gleich mit
dem von Codex geprüften `86f04c1` (`git diff --stat 86f04c1..HEAD` berührte nur
`abnahmen/*`, `docs/werkzeugplan.md`, `werkzeug/LIES-MICH.md`). **Alle zehn
Zeilenanker galten unverändert** — das ist der Grund, warum unten jede
Zeilennummer stimmt statt „ungefähr dort".

---

## Kurzurteil zur Quelle

**Zehn von zehn Befunden bestätigt, keiner widerlegt.** Für dieses Projekt ist
das bemerkenswert: die Hausquote lag bei rund einem Viertel Fehlbefunden.
Codex' Anker waren durchweg exakt (`sonde-messung.html:1398` ist auf die Zeile
genau das `z(true, …)`), und die fünf Zahlen seiner `offsetHeight`-Tabelle
stimmten alle.

**Drei Zahlen von ihm waren trotzdem falsch** — nachgemessen, nicht erschlossen:

| Codex sagt | gemessen | warum |
|---|---|---|
| Maßstab `.46 → 1 → .46` | **`.52 → 1 → .52`** | `.46` gilt erst unter `@media (max-width:1500px)`; die Probe fährt 1600 px |
| Standband **112,75 px** hoch | **94,75 px** | — |
| verdeckt von „`#band` oder `#schirm`" | `band`×382, Panel `t`×400, `fl`×120, BUTTON×18 | `#schirm` traf an keinem der 920 Stichpunkte zu |

Und eine Aussage war zu scharf: „Discard **und Undo** ändern keinen fachlichen
Zustand". Discard tat wirklich nichts; **Undo wirkte sichtbar** (angewandt →
Vorschlagszeile zurück). Tot war Undo nur im Ruhezustand — was der schärfere
Befund ist und unten als E3 geführt wird.

---

## Die vierzehn Punkte

| # | Befund | Zustand | Commit |
|---|---|---|---|
| **1** | P1 · Export verliert die Messwerte | **gefixt** | `bf6c43e` |
| **2** | P1 · Maßstabs-Gegenprobe misst zweimal denselben Maßstab | **gefixt** | `86515fc` |
| **3** | P1 · Zustandsmatrix übersieht die Bypass-Reste | **gefixt** (Probe `9643db9`, Blatt `a6a67bb`) | |
| **4** | P1 · Grenzfall-Gate hart auf `true` | **gefixt** | `a79d842` |
| **5** | P2 · erfundene Parametergrenzen + Lesbarkeitsschwelle | **gefixt** (als Annahme ausgewiesen) · **Sachfrage offen** | `d2674d1` |
| **6** | P2 · Draft/Discard/Undo/Neutralise keine Zustandsmaschine | **gefixt** | `4d882c1` |
| **7** | P2 · `offsetHeight` enthält die zwei Rahmenpixel | **gefixt** | `fb41511` |
| **8** | P2 · Standband existiert, ist aber unsichtbar | **gefixt** | `f960934` |
| **9** | P2 · Kommentar behauptet Breite, gemessen ist Höhe | **gefixt** | `633a6c1` |
| **10** | P3 · `−0.0 dB` und Reiterzustand `apply` | **gefixt** | `4d882c1` |
| **E1** | An/Aus-Punkt bleibt bei Bypass `●` | **WIDERLEGT** — bewusst so | `a6a67bb` |
| **E2** | `slotStreifenEinzeilig()` trägt denselben Defekt wie `slotStreifen()` | **gefixt** | `a6a67bb` |
| **E3** | History-Undo ist im Ruhezustand ein toter Knopf | **gefixt** | `4d882c1` |
| **E4** | `pruefen.mjs`-Gegenprobe deckt den neuen Riegel nicht ab | **gefixt** | `f960934` |
| **E5** | Die Überlaufwarnung verschiebt, was sie misst | **gefixt** | `bf6c43e` |

E1–E5 sind eigene Funde dieser Runde, in keinem Codex-Befund enthalten.

### Der einzige widerlegte Punkt: E1

Ich hatte den An/Aus-Punkt als dritte Bypass-Restanzeige geführt — bei Bypass
bleibt er ein gefüllter `●` und wird nur ausgegraut. Beim Schreiben des Fixes
hat sich das als falsch erwiesen: der Punkt sagt *„dieses Band ist
eingeschaltet"*, und das stimmt auch bei Bypass, denn überbrückt ist das
**Gerät**, nicht das Band. Ihn auf `○` zu stellen wäre die Lüge in die andere
Richtung — genau die Zwei-Begriffe-Falle aus `docs/sondenplan.md` §6
(„Bypass" (Gerät) gegen „an/aus" (Band)). Die Begründung steht jetzt im Code,
damit niemand ihn „nachbessert".

### E5 — von der neuen Export-Probe im ersten Lauf gefunden

`.fenster.schnitt .leib{border-bottom:2px}`: die Überlaufwarnung nahm dem
Inhalt zwei Pixel weg — **nachdem** gemessen war. Das Blatt meldete 23 px und
hatte danach 25. Ein Messgerät, das sein Messobjekt verschiebt, gibt eine Zahl
aus, die nie gegolten hat. **Das erklärt Codex' Spanne „rund 23–25 px":** er
hat beide Zahlen gesehen und den Widerspruch für Rauschen gehalten. Umgestellt
auf `box-shadow: inset` — sieht gleich aus, kostet keinen Platz. Gemessen:
gemeldet 23, lebendig 23, über drei Neuzeichnungen stabil.

---

## Was sich an den Zahlen geändert hat

Alle Kurvenzahlen lagen um 2 px zu hoch (Befund 7). Nachgezogen in
`docs/sondenplan.md` §4 und `werkzeug/LIES-MICH.md` — **nachgemessen, nicht
subtrahiert**:

| | vorher | jetzt |
|---|---:|---:|
| A · Kurvenfeld | 3 px | **1 px** |
| C · Kurvenfeld | 73 px | **71 px** |
| D · Kurvenfeld | 68 px | **66 px** |
| E · Kurvenfeld | 187 px | **185 px** |
| Formfaktor Active | 7 px (0,3/dB) | **5 px (0,2/dB)** |
| „Bänder geben nach" | 85 px | **83 px** |

Das Urteil wird dadurch nicht weicher, sondern härter. **A und B bleiben
ausgeschlossen**, jetzt mit 1 px statt 3.

---

## Die Prüfung ist von 12 auf 15 Proben gewachsen

Drei neue, jede zu einem Befund, der sonst still zurückkommen könnte:

- **`export`** — trägt das JSON die Messung? Verglichen gegen den **lebenden**
  DOM-Zustand, nicht gegen die `dataset`-Felder, aus denen der Export gebaut
  ist. Hat E5 im ersten Lauf gefunden.
- **`annahmen`** — ist jede frei gesetzte Zahl im Blatt **sichtbar** als
  Testannahme benannt? Mit Gegenprobe (Hinweis kurz auf `visibility:hidden`).
- **`maschine`** — ändert **jeder lebende Griff** etwas Sichtbares? Fährt
  offen→verworfen→offen→angewandt→offen, neutralisiert und nimmt zurück,
  klickt den Verlauf leer, hält Audition mit echtem `pointerdown`/`pointerup`.

Dazu vier statt drei Gegenproben in `pruefen.mjs` (neu: verdecktes Standband)
und zwei neue eingebaute Gegenproben (Maßstab, Annahmen).

**Jede neue Prüfung wurde zum Scheitern gebracht, bevor sie als bestanden
gilt:**

- Befund 4: das neue Tor war beim Bau sofort rot — 24 von 290 Zellen. Der
  Blattfix kam als eigener Commit, damit das Rot aktenkundig ist.
- Befund 3: die erweiterte Zustandsmatrix meldete 10 Abweichungen, bevor das
  Blatt repariert wurde (`9643db9` ist absichtlich ein roter Commit).
- Befund 2: künstlich maßstabsabhängige Maße injiziert — gemeldet und geheilt.
- Befund 6: vier künstliche Rückfälle in den Browser injiziert (Discard ohne
  Wirkung, statischer Verlauf, `−0.0 dB`, Undo lebendig bei leerem Verlauf) —
  **4/4 gemeldet**. Der zweite Anlauf war nötig, weil das erste Falsifikat
  selbst falsch war.

---

## Abnahmelauf, roh

```
node werkzeug/pruefung/pruefen.mjs
  10/10 Blaetter heil · formfaktor 920/920 Standband-Stichpunkte sichtbar
  (vorher 0/920)

node werkzeug/pruefung/pruefen.mjs --gegenprobe
  GEGENPROBE: 4/4 kuenstliche Fehler erkannt
  ok  verdecktesband.html wird gemeldet: Standband nur zu 0 % sichtbar
      (0/560 Stichpunkte), verdeckt von DIV×560

node werkzeug/pruefung/sondenprobe.mjs
  ALLES SAUBER (15 Proben)
  zahlen      5 Anordnungen gegengerechnet, 5 bei echtem Massstabswechsel
              0.52→1 geprueft, Gegenprobe erkannt und geheilt
  grenzfall   platz = Vorgabe · ehrlichkeit = 290 Zellen tot,
              103 EQ-Zellen leben in der Gegenstellung
  zustaende   16 Grenzfaelle x 5 Anordnungen = 80 Prueflinge, alle Zellformen
  kachel      Ueberlauf 23 px hoch / 0 px breit — der Engpass ist die HOEHE
  export      v2, Active Kurve 5 px (passt, aber die Kurve ist unlesbar),
              Kachel 23 px hoch (laeuft ueber)
  annahmen    7 frei gesetzte Zahlen sichtbar benannt, Sachquelle genannt
  maschine    offen→verworfen→offen→angewandt→offen, Verlauf leergeklickt,
              Halten per pointerdown/-up
```

---

## Selbstaudit nach dem letzten Commit

Am **gerenderten Blatt** durchgesehen, nicht am Diff — und das hat sich
gelohnt: zwei Texte behaupteten noch, was nicht mehr gemessen wird.

- Der Kopftext nannte „längste Werte" als Teil des Platz-Grenzfalls, während
  das neue Tor sie bewusst **nicht** einfordert. Korrigiert, mit der Messung
  dazu.
- Die Waage-Bemerkung zitierte Q 24.00 · −60,0 dB · 3000 ms ohne Kennzeichnung
  — genau der Weg, auf dem aus einer Annahme „nach der Spezifikation" wird.

Beides steht in `542b162`. Diese beiden Zeilen hatte eine **parallele
Claude-Sitzung** aus dem noch nicht committeten Arbeitsstand aufgesammelt und
als „verwaiste Änderung" gelandet, während hier noch gemessen wurde. Inhaltlich
byte-gleich mit dem, was hier geschrieben wurde, mit eigener Blattprüfung davor
— nichts verloren, nur die Zuordnung stimmt nicht. Festgehalten, damit
niemand den Commit später für eine fremde Änderung hält.

**Zwei Dinge geprüft und absichtlich NICHT geändert:**

- Die Waage schützt nur die Zeile „längste Werte" mit `ohneSchalter`; alle
  anderen zeigen den aktuellen Zustand. Das ist gewollt („was diese Runde
  wirklich anzeigt"). Der Streifen-Fix macht die Streifen jetzt **konsistent**
  dazu, nicht inkonsistent.
- `formfaktor.html` rendert dieselben Slot-Zeilen von Hand, inklusive `dyn`.
  Dort gibt es aber gar keinen Bypass-Zustand — „Bypass off" ist eine feste
  Beschriftung. Kein Zwilling zu Befund 3.

**Kosten gemessen statt geschätzt:** `zeichnen()` 107,3 → 119,2 ms Median
(+11 %) durch die zwei zusätzlichen Ehrlichkeits-Stellungen auf der versteckten
Bühne. Für eine klickgetriebene Messbank vertretbar; für eine Oberfläche, die
pro Frame zeichnet, wäre es das nicht.

**Sichtprüfung des Fußes gerendert:** Hinweisfeld 420 × 79 px, lesbar,
`fussUeberlauf` 0.

---

## Was offen bleibt — und warum es niemand nebenbei entscheidet

1. **Die Bereichsgrenzen der Bandparameter.** Die Spezifikation nennt genau
   eine Zahl: „Gain (manuell bis ±12 dB)". Q 24 · Schwelle −60 dB · Bereich
   −18 dB · Attack 300 ms · Hold 500 ms · Release 3000 ms sind **frei
   gesetzt**. Für den Platzbedarf ist das folgenlos (nachgemessen: mit
   „längsten" und mit realistischen Werten sind alle fünf Anordnungen auf den
   Pixel identisch). Es bleibt eine Produktfrage und gehört in
   `docs/oberflaechen-spezifikation.md`, nicht in einen Codekommentar.
2. **Die Lesbarkeitsschwelle 15,4 px.** Die Herleitung stimmt (11 px Schrift ×
   1,4 Zeilenhöhe). Unbewiesen bleibt die Gleichsetzung „eine Kurve muss so
   hoch sein wie eine Textzeile, um ablesbar zu sein" — eine Kurve ist kein
   Buchstabe. Zu beweisen nur durch einen Sehversuch am 1:1-Blatt.
3. **Wer gibt bei Platzmangel nach — Kurve oder Bänder?** Unverändert offen,
   beide Antworten vorgerechnet (`sondenplan.md` §4).
4. **Die Aufgabenliste nach Häufigkeit** ist weiter Entwurf (`sondenplan.md`
   §3). Ohne sie kein Wireframe.
5. **Greif-Affordanzen sind nicht vermessen.** Alle Zeilen sind reine
   Textzeilen; mit echten Bedienelementen fallen alle Kurvenzahlen um rund
   55 px. Das ist die wichtigste offene Annahme des ganzen Plans.

Punkte 1 und 2 stehen jetzt sichtbar im Blatt und als Annahme 5 und 6 in
`docs/sondenplan.md` §4 — sie können nicht mehr unbemerkt als „nach der
Spezifikation" zitiert werden.
