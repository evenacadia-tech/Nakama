# 2026-09-02 — Technische UI-Architektur, Fünferblock 05 (delegierte Ableitung, gemeinsame Regeln)

## Status

**Zwei Entscheidungen abgeleitet, gemeinsamer Umbau mit Block 04.** Grundlage
ist derselbe User-Auftrag vom 02.09.2026 wie in
[`2026-09-02-technische-ui-architektur-fuenferblock-04.md`](2026-09-02-technische-ui-architektur-fuenferblock-04.md):

> „ich habe gerade keine zeit, kannst du nicht die restlichen offenen UX
> entscheidungen anhand von best practice beispielen lösen ? es gibt ja
> manuals , handbücher mit denen du gute menschliche Bedienprinzipien super
> nachvollziehen und anwenden kannst“

Der Block bleibt mit zwei Entscheiden kürzer als fünf, weil damit die
offenen Bedienfragen aus Blueprint Abschnitt 15 und dem Skizzen-Leseblatt
erschöpft sind; ein Block wird nicht mit erfundenen Fragen aufgefüllt.

## 1/2 — Tastatur, Fokus und Radbesitz über beide Flächen

**Belegte Muster.** W3C-APG *Tabs*: `Left/Right Arrow` wechseln den Fokus
zwischen den Reitern, `Home/End` springen an Anfang und Ende, „Space or
Enter: Activates the tab if it was not activated automatically on focus“;
`aria-selected` markiert den aktiven Reiter. W3C-APG *Disclosure*: `Enter`
und `Space` schalten den Inhalt, `aria-expanded` und `aria-controls` binden
Knopf und Inhalt. W3C-APG *Menu Button*: Öffnen setzt den Fokus in das
erste Element; nach dem Schließen kehrt der Fokus zum Knopf zurück. Pro-Q 4:
„Perhaps the easiest way to make adjustments is by using the mouse wheel
when you hover over a knob“ (unter Windows nach einem Klick in die
Oberfläche), `Shift` für Feinschritte, `Ctrl`-Klick setzt auf den
Standardwert zurück, Doppelklick öffnet die Texteingabe mit Kurzformen wie
„1k“; im Display ändert das Rad die Güte der ausgewählten Bänder.
Kirchhoff-EQ: „drag or use mouse wheel to adjust“ an jedem Wertknopf, Rad
auf dem Typ-Symbol wechselt den Filtertyp. Der Entscheid vom 23.08.2026:
Das Rad wechselt die Sonde nur, wenn nichts per Klick markiert ist.

**Entscheidung für beide Gen-Flächen:**

- **Kopf-Umschalter = Tabs-Muster mit manueller Aktivierung.** `Tab` führt
  auf den aktiven Reiter; `Left/Right` bewegen den Fokus zwischen
  `OVERVIEW` und `EQ`, `Home/End` an die Enden; `Enter` oder `Space`
  wechseln die Fläche. Der Wechsel folgt nicht dem Fokus, weil er eine
  ganze Fläche tauscht.
- **Jede Disclosure folgt einer Regel:** `Enter`/`Space` öffnen und
  schließen; der Einstieg trägt `aria-expanded` und `aria-controls`;
  `Escape` schließt das oberste nichtmodale Panel und gibt den Fokus an
  seinen Einstieg zurück (Global-Sektion an `GLOBAL`, Preset-Liste an
  `PRESET`, Band-Panel an den Bandpunkt, Kopie-Wahl an `COPY →`). Eine
  laufende Texteingabe verwirft `Escape` zuerst (Block 01, 4/5).
- **Radbesitz:** Das Rad gehört dem Wert, über dem der Zeiger steht: über
  einem Wertefeld ändert es diesen Wert in Schritten, mit `Shift` fein;
  über dem Graphen mit ausgewähltem Band ändert es die Güte des Bands; über
  dem Graphen ohne Auswahl wechselt es die Sonde (23.08.2026); über der
  Sources-Liste scrollt es die Liste. `Ctrl`-Klick auf ein Wertefeld setzt
  den Standardwert. Doppelklick öffnet die Texteingabe; `Enter` übernimmt,
  `Escape` verwirft.
- **Fokusordnung:** Kopf, Zielleiste beziehungsweise Sources-Spalte,
  Arbeitsfläche, untere Zeile. Ein geöffnetes Panel liegt in der Ordnung
  direkt hinter seinem Einstieg.
- Die Blueprint-Hypothese **[A] Radbesitz** aus Abschnitt 4.2 wird damit
  **[U]** im Sinn dieser delegierten Ableitung.

## 2/2 — Fehler- und Degradationszustände über beide Flächen

**Belegte Muster.** Nielsen, Heuristik 1: „keep users informed about what
is going on“; NN/g: Indikatoren sind kontextuell und passiv, Validierungen
hängen an der Eingabe, Benachrichtigungen gelten Ereignissen ohne
Nutzerauslösung; „a critical error should never use a dismissible toast
notification, nor should routine feedback interrupt users with modal
dialogs“. NN/g-Fehlerregeln: nahe an der Quelle, redundant und zugänglich,
konstruktiv, Eingabe erhalten. Pro-Q 4 zeigt Bypass eines Bands als
„dimmed in the display and a red light glows in the bypass button“, also
doppelt.

**Entscheidung für beide Gen-Flächen und Probeeq:**

- **Systemwahrheit steht im Kopf**, dauerhaft und passiv: Verbindung
  (`SESSION LINKED` / `SESSION LINK LOST`), Autorität und Revision. Sie
  wechselt nie in einen Dialog.
- **Ein degradiertes Objekt bleibt an seinem Ort und trägt seinen
  Zustand** (Quelle `STALE`, `PARTIAL`, `LOW SIGNAL`; Befund `MORE DATA`,
  `BLOCKED · LINK`; Band `OFF`; Ziel `LINK LOST`). Nichts verschwindet,
  nichts wird nur farblich markiert.
- **Validierung sitzt am Feld:** Eine unbrauchbare Eingabe wird verworfen,
  der alte Wert bleibt, und eine kurze Zeile am Feld sagt es (`WIDTH ·
  KEPT 100 %`); keine Sperre, kein Dialog.
- **Ereignisse ohne Nutzerauslösung** (Link-Verlust, ausgelaufener
  Kandidat, blockierte Fernänderung) erscheinen als Hinweiszeile am
  betroffenen Ort (Graph-Hinweis, Transaktionszeile, Probeeq-Zeile 2) mit
  dem nächsten Schritt; sie sind nicht wegklickbar, sondern enden mit dem
  Zustand.
- **Modale Dialoge gibt es nicht.** Die einzige zweistufige Handlung ist
  die Bestätigung einer überschreibenden Aktion (Preset überschreiben,
  Kurve kopieren, `APPLY` als 10-s-Kandidat), und sie läuft im selben
  Körper wie die Aktion.

## Quellenledger

Siehe das Ledger von Fünferblock 04; zusätzlich für diesen Block dieselben
W3C-APG-Seiten (Tabs, Disclosure, Menu Button, Combobox) und die
NN/g-Artikel (Heuristiken, Error-Message Guidelines, Indicators/Validations/
Notifications).

## Gemeinsamer Umbau und Sichtbeleg (Block 04 und 05)

Die Skizze `design/skizze/nakama-ui-technical-sketch.html` setzt alle sieben
Ableitungen um. Geprüft am 02.09.2026 mit Playwright und dem System-Chrome
(headless, Viewport 1500×900): das Blockskript mit 65 Prüfpunkten und das
Regressionsskript von Block 03 mit 42 Prüfpunkten, beide PASS, Konsole ohne
Fehler oder Warnungen.

| Punkt | Messung |
|---|---|
| Sources-Spalte | 180 px breit, 16 Quellen in Mixer-Reihenfolge, Liste scrollt in der Spalte; Filter `gui` → `2 MATCH · 16`, `Enter` wählt `GUITAR L`, `Escape` leert |
| Quelle ohne Befund | Zustand `NO FINDING`, `SEND DRAFT` gesperrt |
| Kopf-Tabs | `ArrowRight` bewegt den Fokus ohne Wechsel, `Home`/`End`, `Enter` wechselt; Fokus landet auf dem Reiter der neuen Fläche |
| Halten | beide Flächen `HOLD TO AUDITION`, Knopf auf Fläche 2 128×30, Transaktionszeile ohne Überlauf |
| `Delete` auf Bandpunkt | entfernt B4, Fokus auf `UNDO` (58×30); Undo stellt wieder her, Redo entfernt erneut |
| Global-Sektion | `PRESET` zeigt `USER 04`, Marker bei Änderung; `REV 128 · DRAFT OPEN` als Leseanzeige; kein `HISTORY`-Knopf; Öffnen schließt das Band-Panel |
| Validierung | `abc` in WIDTH → `WIDTH · NOT A NUMBER, KEPT 100 %`, gültige Eingabe löscht die Notiz; `Ctrl`-Klick setzt den Standard; Rad über MIX ändert um 5 |
| Preset-Liste | ersetzt den Global-Körper, Fokus im Filter; `vocal` → ein Treffer, `Enter` lädt `VOCAL PRESENCE`; `SAVE` bei Factory gesperrt; `SAVE` bei `USER 01` fragt `OVERWRITE USER 01 WITH THE CURRENT STATE?`; `SAVE AS` bietet `USER 05` an und legt es an; `Escape` schließt Liste → `PRESET`, dann Global → `GLOBAL` |
| Rad über dem Graphen | mit offenem Panel ändert es Q, ohne Auswahl wechselt es die Sonde |
| Kurven-Kopie | `COPY →` ersetzt die Zielleiste, Quelle ist kein Ziel, gewähltes Ziel bleibt gedrückt sichtbar, `REPLACES THE PIANO CURVE`, `CONFIRM COPY` → Hinweis `COPIED BASS CURVE TO PIANO`; Undo/Redo; `Escape` zurück zu `COPY →` |
| Probeeq | 600×92 mit EQ an und aus; Zeile 2 bei EQ aus `EQ OFF · PASSIVE MEASUREMENT · SIGNAL · FRESH 1.2 s`; Link-Verlust `BROKER OFFLINE` in Zeile 1, `LOCAL CONFIRMED · REV 128 · REMOTE LOCKED · EQ, BYPASS, MIX STAY LOCAL` in Zeile 2; EQ, Bypass, Mix bedienbar; Zeilen ohne Überlauf |

Belege unter `design/skizze/belege/` mit Präfix `2026-09-02-b0405-`
(Sources-Filter, Preset-Liste, Kopie-Bestätigung, Validierung, Probeeq bei
EQ aus, Probeeq bei Link-Verlust).

Technische Konsequenzen des Umbaus, keine neuen Entscheide:

- **Ein Panel über dem Graphen.** Öffnen der Global-Sektion schließt das
  Band-Panel, Öffnen eines Band-Panels schließt die Global-Sektion. Das
  folgt aus der `Escape`-Regel „oberstes nichtmodales Panel“: Es gibt genau
  eines.
- Die Probeeq-Kachel wurde von 580 auf 600 px verbreitert, weil `BROKER
  OFFLINE` in Zeile 1 sonst 28 px überlief; Abstände 7 px, Mix-Regler 76 px.
  Maße bleiben Dichteprüfung.
- Die Kopie wirkt in der Skizze nur als Hinweis und Undo-Eintrag; die
  Ziele teilen sich dort denselben Demo-Bandsatz. Der Datenweg ist S29–31.
- Der Demo-Preset-Bestand (drei mitgelieferte, vier eigene) und der
  Wortlaut der Hinweise sind Dichteprüfung.

Nebenbefunde der Prüfung, im selben Umbau behoben:

1. `Ctrl`-Klick setzte den Wert zurück, das fokussierte Feld zeigte aber
   den alten Text weiter; jetzt wird das Feld nach Rücksetzen und Rad
   ausdrücklich nachgeführt.
2. `SAVE AS` ließ die Preset-Liste offen und schickte den Fokus auf den
   verdeckten `PRESET`-Knopf; die Liste schließt jetzt beim Sichern.
3. Die Kopie-Zielwahl kannte das aktuelle Ziel nicht als Eintrag; jetzt
   sind alle Ziele gelistet und das aktuelle wird dynamisch ausgeblendet.
4. `Escape` in der Kopie-Zielwahl griff nur bei Fokus in der Zielwahl;
   jetzt gilt es in der ganzen Zielleiste, solange die Wahl offen ist.

## Sessionende 02.09.2026 (Laptop)

Offen geblieben: die Produktnamen der beiden Gen-Flächen (Karte U23) und
der Wechsel in die visuelle Phase in Figma, beides Entscheide des Users.
Zwei abgeleitete Konsequenzen stehen zur Bestätigung: Probeeq-Zeile 2
bleibt immer sichtbar (feste Fensterhöhe) und die Kachel ist 600 px breit.
Alles ist committet und gepusht; PR2 R1 läuft parallel auf dem PC.
